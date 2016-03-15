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


#include <vector>
#include <tuple>

#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>
#include <thread>
#include <pthread.h>

using namespace std;


class MuzzleyConfigManager {
  public:

    MuzzleyConfigManager();

    ~MuzzleyConfigManager();

    void print_muzzley_subscription_pos(int pos);
    void print_muzzley_subscription_vector();
    int get_muzzley_subscription_vector_size();
    bool add_muzzley_subscription(string profile, string channel, string component);
    string get_muzzley_subscription_profileid(int pos);
    string get_muzzley_subscription_channelid(int pos);
    string get_muzzley_subscription_componentid(int pos);
    int get_muzzley_subscription_profileid_pos(string profileid);
    int get_muzzley_subscription_channelid_pos(string channelid);
    int get_muzzley_subscription_componentid_pos(string componentid);
    bool del_muzzley_subscription_pos(int pos);

    string read_muzzley_devicekey_file(string filepath);
    void write_muzzley_devicekey_file(string filepath, string deviceKey);

  private:

    enum MUZZLEY_SUBSCRIPTION {
        PROFILE=0,
        CHANNEL=1,
        COMPONENT=2,
    };

    typedef std::tuple <string, string, string> muzzley_subscription;   
    vector<muzzley_subscription> muzzley_subscription_vector;

};

