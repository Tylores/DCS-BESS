/******************************************************************************
 * Copyright AllSeen Alliance. All rights reserved.
 *    Copyright (c) V2 Systems, LLC.  All rights reserved.
 *
 *    All rights reserved. This program and the accompanying materials are
 *    made available under the terms of the Apache License, Version 2.0
 *    which accompanies this distribution, and is available at
 *    http://www.apache.org/licenses/LICENSE-2.0
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
**********************************************************/

#ifndef SERVERLISTENER_HPP_INCLUDED
#define SERVERLISTENER_HPP_INCLUDED

class ServerListener : public ajn::MessageReceiver,
                       public ajn::Observer::Listener,
                       public ajn::ProxyBusObject::PropertiesChangedListener {
    static const char* props[];

public :
    // member variables
    ajn::BusAttachment* bus_;
    ajn::Observer* obs_;
    const char* server_interface_;
    int price_;
    unsigned int time_;

public :
    // member functions
    ServerListener (ajn::BusAttachment* bus,
                    ajn::Observer* obs,
                    const char* server_name);
    virtual void ObjectDiscovered (ajn::ProxyBusObject& proxy);
    virtual void ObjectLost (ajn::ProxyBusObject& proxy);
    virtual void PropertiesChanged (ajn::ProxyBusObject& proxy,
                                    const char* interface_name,
                                    const ajn::MsgArg& changed,
                                    const ajn::MsgArg& invalidated,
                                    void* context);
};

#endif // SERVERLISTENER_HPP_INCLUDED
