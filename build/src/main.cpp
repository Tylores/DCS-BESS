/******************************************************************************
 *    Copyright (c) Open Connectivity Foundation (OCF), AllJoyn Open Source
 *    Project (AJOSP) Contributors and others.
 *
 *    SPDX-License-Identifier: Apache-2.0
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Copyright (c) Open Connectivity Foundation and Contributors to AllSeen
 *    Alliance. All rights reserved.
 *
 *    Copyright (c) V2 Systems, LLC.  All rights reserved.
 *
 *    Permission to use, copy, modify, and/or distribute this software for
 *    any purpose with or without fee is hereby granted, provided that the
 *    above copyright notice and this permission notice appear in all
 *    copies.
 *
 *    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL
 *    WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED
 *    WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE
 *    AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 *    DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR
 *    PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER
 *    TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR
 *    PERFORMANCE OF THIS SOFTWARE.
******************************************************************************/

// INCLUDE
#include <iostream>     // cout, cin
#include <thread>       // thread, join
#include <chrono>       // now, duration
#include <map>
#include <string>
#include <vector>

#include "include/ts_utility.hpp"
#include "include/aj_utility.hpp"
#include "include/DistributedEnergyResource.hpp"
#include "include/ServerListener.hpp"
#include "include/SmartGridDevice.hpp"

using namespace std;
using namespace ajn;

bool done = false;  //signals program to stop

// Help
// - CLI interface description
static void Help () {
    printf ("\n\t[Help Menu]\n\n");
    printf ("> q            quit\n");
    printf ("> h            display help menu\n");
    printf ("> i <watts>    import power\n");
    printf ("> e <watts>    export power\n");
    printf ("> p            print properties\n");
} // end Help

// TerminalHelp
// - command line interface arguments during run, [] items have default values
static void CommandLineHelp (const string& kArg) {
    const char* s = kArg.c_str();
    cout << "\n[Usage] > " << s << " -c file [-t ] [-h help]" << endl;
    cout << "\t -h \t help" << endl;
    cout << "\t -c \t configuration filename" << endl;
    cout << "\t -t \t time multiplier. (default = 300 i.e, 5 minutes)" << endl;
} // end TerminalHelp

// Command Line Parse
// - method to parse program initial parameters
static map<string, string> CommandLineParse (int argc, char** argv) {
    // parapeter tokens
    enum {HELP, CONFIG, TIME};
    vector<string> tokens = {"-h", "-c", "-t"};
    string name = argv[0];

    // parse tokens
    map<string, string> parameters;
    string token, parameter;

    for (int i = 1; i < argc; i = i+2){
        token = argv[i];

        if (token.compare(tokens[HELP]) == 0) {
            CommandLineHelp(name);
            exit(EXIT_FAILURE);
        } else if (token.compare(tokens[CONFIG]) == 0) {
            parameter = argv[i+1];
            parameters["config"] = parameter;
        } else if (token.compare(tokens[TIME]) == 0) {
            parameter = argv[i+1];
            parameters["time"] = parameter;
        } else {
            const char* s = token.c_str();
            cout << "[ERROR] : Invalid argument: " << s << endl;
            CommandLineHelp(name);
            exit(EXIT_FAILURE);
        }
    }
    return parameters;
} // end CommandLineParse

// Command Line Interface
// - method to allow user controls during program run-time
static bool CommandLineInterface (const string& input, 
                                  DistributedEnergyResource *DER) {
    // check for program argument
    if (input == "") {
        return false;
    }
    char cmd = input[0];

    // deliminate input string to argument parameters
    vector <string> tokens;
    stringstream ss(input);
    string token;
    while (ss >> token) {
        tokens.push_back(token);
    }

    switch (cmd) {
        case 'q':
           return true;

        case 'i': {
            try {
                DER->SetImportWatts(stoul(tokens[1]));
            } catch(...) {
                cout << "[ERROR]: Invalid Argument." << endl;
            }
            break;
        }

        case 'e': {
            try {
                DER->SetExportWatts(stoul(tokens[1]));
            } catch(...) {
                cout << "[ERROR]: Invalid Argument." << endl;
            }
            break;
        }

        case 'p': {
            cout << "\n\t[Properties]\n\n";
            cout << "Export Energy:\t" << DER->GetExportEnergy () << endl;
            cout << "Export Power:\t" << DER->GetExportPower () << endl;
            cout << "Import Energy:\t" << DER->GetImportEnergy () << endl;
            cout << "Import Power:\t" << DER->GetImportPower () << endl;
            break;
        }

        default: {
            Help();
            break;
        }
    }

    return false;
}  // end Command Line Interface

void ResourceLoop (DistributedEnergyResource *DER) {
    unsigned int time_remaining, time_past;
    unsigned int time_wait = 500;
    auto time_start = chrono::high_resolution_clock::now();
    auto time_end = chrono::high_resolution_clock::now();
    chrono::duration<double, milli> time_elapsed;

    while (!done) {
        time_start = chrono::high_resolution_clock::now();
            // time since last control call;
            time_elapsed = time_start - time_end;
            time_past = time_elapsed.count();
            DER->Loop(time_past);
        time_end = chrono::high_resolution_clock::now();
        time_elapsed = time_end - time_start;

        // determine sleep duration after deducting process time
        time_remaining = (time_wait - time_elapsed.count());
        time_remaining = (time_remaining > 0) ? time_remaining : 0;
        this_thread::sleep_for (chrono::milliseconds (time_remaining));
    }
}  // end Resource Loop

// Main
// ----
int main (int argc, char** argv) {
    cout << "\nStarting Program...\n";
    cout << "\n\tLoading parameters...\n";

    if (argc == 1) {
        // this means no arguments were given
        string name = argv[0];
        CommandLineHelp(name);
        return EXIT_FAILURE;
    }
    map<string, string> parameters = CommandLineParse(argc, argv);

    cout << "\n\tMapping configuration file...\n";
    string config_file = parameters.at("config");
    tsu::config_map ini_map = tsu::MapConfigFile(config_file);

    cout << "\n\tStarting AllJoyn...\n";

    try {
        AllJoynInit();
    } catch (exception &e) {
        cout << "[ERROR]: " << e.what() << endl;
        return EXIT_FAILURE;
    }

    #ifdef ROUTER
        try {
            AllJoynRouterInit();
        } catch (exception &e) {
            cout << "[ERROR]: " << e.what() << endl;
            return EXIT_FAILURE;
        }
    #endif // ROUTER

    cout << "\n\t\tCreating message bus...\n";
    string app_name = tsu::GetSectionProperty(ini_map, "AllJoyn", "app");
    bool allow_remote = true;
    BusAttachment *bus_ptr = new BusAttachment(app_name.c_str(), allow_remote);
    assert(bus_ptr != NULL);

    cout << "\n\t\tCreating about object...\n";
    AboutData about_data("en");
    AboutObj *about_ptr = new AboutObj(*bus_ptr);
    assert(about_ptr != NULL);

    cout << "\n\t\tEstablishing session port...\n";
    aj_utility::SessionPortListener SPL;
    string port_number = tsu::GetSectionProperty(ini_map, "AllJoyn", "port");
    SessionPort port = stoul(port_number);

    cout << "\n\t\tSetting up bus attachment...\n";
    QStatus status = aj_utility::SetupBusAttachment (ini_map,
                                                     port,
                                                     SPL,
                                                     bus_ptr,
                                                     &about_data);

    if (status != ER_OK) {
        delete about_ptr;
        delete bus_ptr;
        return EXIT_FAILURE;
    }

    cout << "\n\t\tLooking for resource...\n";

    DistributedEnergyResource *der_ptr = 
        new DistributedEnergyResource (ini_map["DER"]);

    cout << "\n\t\tCreating observer...\n";
    string server_interface = tsu::GetSectionProperty(ini_map,
                                                      "AllJoyn",
                                                      "server_interface");
    const char *server_name = server_interface.c_str();
    Observer *obs_ptr = new Observer(*bus_ptr, &server_name, 1);

    cout << "\n\t\tCreating listener...\n";
    ServerListener *listner_ptr = new ServerListener(bus_ptr,
                                                     obs_ptr,
                                                     server_name);
    obs_ptr->RegisterListener(*listner_ptr);

    cout << "\n\t\tCreating bus object...\n";
    string device_interface = tsu::GetSectionProperty(ini_map,
                                                      "AllJoyn",
                                                      "device_interface");
    const char *device_name = device_interface.c_str();

    string path_str = tsu::GetSectionProperty(ini_map, "AllJoyn", "path");
    const char *path = path_str.c_str();

    SmartGridDevice *sgd_ptr = new SmartGridDevice(der_ptr, 
                                                   bus_ptr, 
                                                   device_name, 
                                                   path);

    cout << "\n\t\t\tRegistering bus object...\n";
    if (ER_OK != bus_ptr->RegisterBusObject(*sgd_ptr)){
        printf("\n\t\t\t[ERROR] failed registration...\n");
        delete &sgd_ptr;
        return EXIT_FAILURE;
    }
    about_ptr->Announce(port, about_data);

    cout << "\nProgram initialization complete...\n";
    thread DER(ResourceLoop, der_ptr);

    Help ();
    string input;

    while (!done) {
        getline(cin, input);
        done = CommandLineInterface(input, der_ptr);
        sgd_ptr->SendPropertiesUpdate ();
    }

    cout << "\nProgram shutting down...\n";
    cout << "\n\t Joining threads...\n";
    DER.join();

    cout << "\n\t deleting pointers...\n";
    delete der_ptr;
    delete sgd_ptr;
    delete listner_ptr;
    delete obs_ptr;
    delete about_ptr;
    delete bus_ptr;

    cout << "\n\t Shutting down AllJoyn...\n";
    obs_ptr->UnregisterAllListeners ();

    #ifdef ROUTER
        AllJoynRouterShutdown ();
    #endif // ROUTER

    AllJoynShutdown ();

    return EXIT_SUCCESS;
} // end main
