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

#include "MqttTransport.h"


#define QOS 0

MqttTransport::MqttTransport( TransportListener*    listener) : Transport( listener ){
    char *host = (char*)"cloud-services.muzzley.com";
    int port = 9883;
    int keepalive = 60;
    bool clean_session = true;
    
    mosquitto_lib_init();

    this->mosq_trans = mosquitto_new(NULL, clean_session, listener);
    if(!this->mosq_trans){
        printf("Error: Out of memory.\n");
        return;
    }else{
        mosquitto_message_callback_set(this->mosq_trans, my_message_callback);
        mosquitto_log_callback_set(this->mosq_trans, my_log_callback);
        mosquitto_subscribe_callback_set(this->mosq_trans, my_subscribe_callback);
        mosquitto_connect_callback_set(this->mosq_trans, my_connect_callback);
        
        int _return;
        _return=mosquitto_connect(this->mosq_trans, host, port, keepalive);
        printf("Mosquitto Connect Ack Result: %s\n\n", mosquitto_connack_string(_return));
         
        _return = mosquitto_loop_start(this->mosq_trans);
        printf("Mosquitto Loop Start Ack Result: %s\n\n", mosquitto_strerror(_return));

        this->SetConnectionState(connected);
        //this->GlobalConnectionStateChanged(connected, none);
   
    }
}

MqttTransport::~MqttTransport(){
    int _return = mosquitto_loop_stop(this->mosq_trans, true);
    printf("Mosquitto Loop Stop Ack Result: %s\n\n", mosquitto_strerror(_return));
    mosquitto_lib_cleanup();    
}

Transport::ConnectionError MqttTransport::RunOnce(){
    Transport::ConnectionError err;
    err = none;
    return err;
}

void MqttTransport::StopImpl(){

}

void MqttTransport::SubscribeImpl(const string& topic){
    printf("Subscribing MQTT transport\n\n"); 
    int _return=-1;
    _return=this->subscribe(this->mosq_trans, topic);
    if(_return==MOSQ_ERR_SUCCESS){
        printf("Subscribed: %s successfully\n\n", topic.c_str());   
    }else{
        printf("Failed to subscibe MQTT topic: %s Error: %s \n\n", topic.c_str(), mosquitto_strerror(_return)); 
    }
}

Transport::ConnectionError MqttTransport::SendImpl(const string& topic, const string& message){
    this->publish(this->mosq_trans, topic, message);
    Transport::ConnectionError err;
    err = none;
    return err;
}

void MqttTransport::my_message_callback(struct mosquitto *mosq, void *userdata, const struct mosquitto_message *message){
    
    printf("Processing Received Message!\n");
    printf("Received MQTT Message:\n");
    printf("Topic: %s Message:\n%s\n", (char*)message->topic, (char*)message->payload);
    TransportListener * trans = (TransportListener*) userdata;

    string topic_str = std::string((char*)message->topic);
    std::istringstream _topic(topic_str);
    const std::string topic(_topic.str());

    std::istringstream _iss(std::string((char*) message->payload, message->payloadlen));
    const std::string str(_iss.str());

    if(message->payloadlen){
        trans->MessageReceived((const std::string&)(topic), (const std::string&)(str));
    }else{
        printf("Received Empty Message from topic: %s\n\n", topic.c_str());
    }
    

    fflush(stdout);
}

void MqttTransport::my_connect_callback(struct mosquitto *mosq, void *userdata, int result){   
    printf("Connect Callback result: %s\n", mosquitto_strerror(result));     
}

void MqttTransport::my_subscribe_callback(struct mosquitto *mosq, void *userdata, int mid, int qos_count, const int *granted_qos){
    printf("Subscribed (mid: %d) QOS:%d\n", mid, granted_qos[0]);
    for(int i=1; i<qos_count; i++){
        printf(", %d", granted_qos[i]);
    }
    printf("\n");
}

void MqttTransport::my_log_callback(struct mosquitto *mosq, void *userdata, int level, const char *str){
    //Pring all log messages regardless of level.
    printf("MQTT Log: %s\n", str);   
}

int MqttTransport::publish(struct mosquitto *mosq, std::string _topic, std::string _payload){
    int _return;
    std::string _payload_str = (std::string) _payload;
    mosquitto_publish(mosq, & _return, _topic.data(), _payload_str.length(), (const uint8_t *) _payload_str.data(), QOS, false);
    return _return;
}


int MqttTransport::subscribe(struct mosquitto *mosq, std::string _topic) {
    int _return; 
    mosquitto_subscribe(mosq, & _return, _topic.data(), QOS);
    return _return;
}



