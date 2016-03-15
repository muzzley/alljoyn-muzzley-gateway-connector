/**
 * Copyright (c) 2015, Muzzley
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#pragma once

#include "transport/Transport.h"
#include <mosquitto.h>
#include <stdio.h>
#include <stdint.h>
#include <vector>
#include <string>
#include <sstream>
#include <functional>
#include <iostream>

using namespace std;

class MqttTransport : public Transport{ MqttTransport() : Transport(0) {}

public:

    MqttTransport(TransportListener* listener);
    ~MqttTransport();

private:

    virtual ConnectionError RunOnce();
    virtual void StopImpl();

    virtual void  SetCredentialsImpl(const std::string& username, const std::string& password);
    virtual void  ConnectImpl();

    virtual void SubscribeImpl(const std::string& topic);
    virtual ConnectionError SendImpl(const std::string& topic, const std::string& message);

private:

    static void my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message);
    static void my_connect_callback(struct mosquitto *mosq, void *userdata, int result);
    static void my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos);
    static void my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str);
    static int publish(struct mosquitto *mosq, std::string _topic, std::string _payload);
    static int subscribe(struct mosquitto *mosq, std::string _topic);


};


