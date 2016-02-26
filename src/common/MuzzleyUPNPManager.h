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

#include "MuzzleyUPNP.h"

#include <vector>
#include <tuple>

#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <thread>
#include <pthread.h>

using namespace std;


class MuzzleyUPNPManager {
  public:

    MuzzleyUPNPManager();

    ~MuzzleyUPNPManager();

    void print_muzzley_upnp_advertiser_pos();
    void print_muzzley_upnp_advertiser_vector();
    void get_muzzley_upnp_advertiser_vector_size();
    bool add_muzzley_upnp_advertiser(string interface, string host, int port, string xml_fileName, string xml_filePath,
                                string profileId, string friendlyName, string channelId, string macAddress,
                                string serialNumber, string manufacturer, string manufacturer_Url, string modelName,
                                string modelNumber, string modelDescription, vector <component> components);
        
    string get_muzzley_upnp_advertiser_profileid(int pos);
    string get_muzzley_upnp_advertiser_channelid(int pos);
    string get_muzzley_upnp_advertiser_friendlyname(int pos);
    MuzzleyUPNPAdvertiser* get_muzzley_upnp_advertiser_class(int pos);
    bool del_upnp_advertiser_pos(int pos);
    bool del_upnp_advertiser_profileid(string profile);
    bool del_upnp_advertiser_channelid(string channel);
    void generate_file_muzzley_upnp_advertiser(int pos);
    void start_loop_muzzley_upnp_adveritiser(int pos);
    void stop_loop_muzzley_upnp_adveritiser(int pos);

    void loop();

  private:

    GMainLoop *main_loop;

    enum MUZZLEY_UPNP_ADVERTISER {
        PROFILE=0,
        CHANNEL=1,
        FRIENDLYNAME=2,
        UPNP_ADVERTISER=3,
    };

    typedef std::tuple <string, string, string, MuzzleyUPNPAdvertiser*> alljoyn_event_method;   
    vector<alljoyn_event_method> muzzley_upnp_advertiser_vector;

};

