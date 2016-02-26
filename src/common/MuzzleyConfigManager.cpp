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



#include "MuzzleyConfigManager.h"

using namespace std;

MuzzleyConfigManager::MuzzleyConfigManager(){

}

MuzzleyConfigManager::~MuzzleyConfigManager(){

}

void MuzzleyConfigManager::print_muzzley_subscription_pos(int pos){
    cout << "Subscription#: " << pos+1 << "/" << muzzley_subscription_vector.size() << endl << flush;
    cout << "ProfileID: " << get<PROFILE>(muzzley_subscription_vector[pos]) << endl << flush;
    cout << "ChannelID: " << get<CHANNEL>(muzzley_subscription_vector[pos]) << endl << flush;
    cout << "ComponenTid: " << get<COMPONENT>(muzzley_subscription_vector[pos]) << endl << flush;
    
}

void MuzzleyConfigManager::print_muzzley_subscription_vector(){
    cout << endl << "--- Muzzley Subscriptions: " << muzzley_subscription_vector.size() << " ---" << endl << endl << flush;
    for (unsigned int i = 0; i < muzzley_subscription_vector.size(); i++){
        print_muzzley_subscription_pos(i);
    }
    cout << "---END---" << endl << endl << flush;
}

int MuzzleyConfigManager::get_muzzley_subscription_vector_size(){
    return muzzley_subscription_vector.size();
}

bool MuzzleyConfigManager::add_muzzley_subscription(string profile, string channel, string component){
    try{
        muzzley_subscription_vector.push_back(make_tuple(profile, channel, component));
        cout << "Stored a new Muzzley Subscription" << endl << flush;
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

string MuzzleyConfigManager::get_muzzley_subscription_profileid(int pos){
    try{
        return get<PROFILE>(muzzley_subscription_vector[pos]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return "";
    }
}

string MuzzleyConfigManager::get_muzzley_subscription_channelid(int pos){
    try{
        return get<CHANNEL>(muzzley_subscription_vector[pos]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return "";
    }
}

string MuzzleyConfigManager::get_muzzley_subscription_componentid(int pos){
    try{
        return get<COMPONENT>(muzzley_subscription_vector[pos]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return "";
    }
}

int MuzzleyConfigManager::get_muzzley_subscription_profileid_pos(string profileid){
    try{
        for(unsigned int i=0;i<muzzley_subscription_vector.size();i++){
          if(get_muzzley_subscription_profileid(i)==profileid){
            return i;
          }
        }
        return -1;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return -1;
    }
}

int MuzzleyConfigManager::get_muzzley_subscription_channelid_pos(string channelid){
    try{
        for(unsigned int i=0;i<muzzley_subscription_vector.size();i++){
          if(get_muzzley_subscription_channelid(i)==channelid){
            return i;
          }
        }
        return -1;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return -1;
    }
}

int MuzzleyConfigManager::get_muzzley_subscription_componentid_pos(string componentid){
    try{
        for(unsigned int i=0;i<muzzley_subscription_vector.size();i++){
          if(get_muzzley_subscription_componentid(i)==componentid){
            return i;
          }
        }
        return -1;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return -1;
    }
}

    
bool MuzzleyConfigManager::del_muzzley_subscription_pos(int pos){
    try{
        muzzley_subscription_vector.erase(muzzley_subscription_vector.begin()+pos);
        cout << "Deleted Muzzley subscription on pos#: " << pos << endl << flush; 
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

string MuzzleyConfigManager::read_muzzley_devicekey_file(string filepath){
    ifstream myfile;
    string line;
    myfile.open (filepath.c_str(), std::ifstream::in);
    getline(myfile, line);
    myfile.close();
    return line;    
}

void MuzzleyConfigManager::write_muzzley_devicekey_file(string filepath, string deviceKey){
    ofstream myfile;
    myfile.open (filepath.c_str(), std::ifstream::out);    
    myfile << deviceKey;
    myfile.close();
}
