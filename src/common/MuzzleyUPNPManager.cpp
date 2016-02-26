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


#include "MuzzleyUPNPManager.h"

using namespace std;

MuzzleyUPNPManager::MuzzleyUPNPManager(){

}

MuzzleyUPNPManager::~MuzzleyUPNPManager(){

}

void MuzzleyUPNPManager::print_muzzley_upnp_advertiser_pos(){

}

void MuzzleyUPNPManager::print_muzzley_upnp_advertiser_vector(){

}

void MuzzleyUPNPManager::get_muzzley_upnp_advertiser_vector_size(){

}

bool MuzzleyUPNPManager::add_muzzley_upnp_advertiser(string interface, string host, int port, string xml_fileName, string xml_filePath,
                            string profileId, string friendlyName, string channelId, string macAddress,
                            string serialNumber, string manufacturer, string manufacturer_Url, string modelName,
                            string modelNumber, string modelDescription, vector <component> components){

    for(int i=0;i<muzzley_upnp_advertiser_vector.size();i++){
      if(get<PROFILE>(muzzley_upnp_advertiser_vector[i])==profileId){
        get<UPNP_ADVERTISER>(muzzley_upnp_advertiser_vector[i])->activate_advertiser();
        get<UPNP_ADVERTISER>(muzzley_upnp_advertiser_vector[i])->update_xml_file(interface, host, port, xml_fileName, xml_filePath,
                             profileId, friendlyName, channelId, macAddress, serialNumber, manufacturer, manufacturer_Url, modelName,
                             modelNumber, modelDescription, components);
        return true;
      }
    }

    try{
        MuzzleyUPNPAdvertiser* upnp_advertiser = new MuzzleyUPNPAdvertiser(interface, host, port, xml_fileName, xml_filePath,
                                                                          profileId, friendlyName, channelId, macAddress,
                                                                          serialNumber, manufacturer, manufacturer_Url,
                                                                          modelName, modelNumber, modelDescription, components);
        
       
        muzzley_upnp_advertiser_vector.push_back(make_tuple(profileId, channelId, friendlyName, upnp_advertiser));
        cout << "Stored a new Muzzley UPnP Advertiser" << endl << flush;
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }

}

string MuzzleyUPNPManager::get_muzzley_upnp_advertiser_profileid(int pos){
    try{
        return get<PROFILE>(muzzley_upnp_advertiser_vector[pos]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return "";
    }
}

string MuzzleyUPNPManager::get_muzzley_upnp_advertiser_channelid(int pos){
    try{
        return get<CHANNEL>(muzzley_upnp_advertiser_vector[pos]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return "";
    }
}

string MuzzleyUPNPManager::get_muzzley_upnp_advertiser_friendlyname(int pos){
    try{
        return get<FRIENDLYNAME>(muzzley_upnp_advertiser_vector[pos]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return "";
    }
}

MuzzleyUPNPAdvertiser* MuzzleyUPNPManager::get_muzzley_upnp_advertiser_class(int pos){
    try{
        return get<UPNP_ADVERTISER>(muzzley_upnp_advertiser_vector[pos]);
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        MuzzleyUPNPAdvertiser* ad;
        return ad;
    }
}

bool MuzzleyUPNPManager::del_upnp_advertiser_pos(int pos){
    try{
        get<UPNP_ADVERTISER>(muzzley_upnp_advertiser_vector[pos])->deactivate_advertiser();
        cout << "Deleted UPNP advertiser on pos#: " << pos << endl << flush;
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }
}

bool MuzzleyUPNPManager::del_upnp_advertiser_profileid(string profile){
    try{
        for(unsigned int i=0; i<muzzley_upnp_advertiser_vector.size();i++){
          if(get<PROFILE>(muzzley_upnp_advertiser_vector[i])==profile){
             get<UPNP_ADVERTISER>(muzzley_upnp_advertiser_vector[i])->deactivate_advertiser();
             cout << "Deleted UPNP advertiser for profileid: " << profile << endl << flush;
          }
        }
        return true;
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
        return false;
    }    
}

bool MuzzleyUPNPManager::del_upnp_advertiser_channelid(string channel){
    try{
          for(unsigned int i=0; i<muzzley_upnp_advertiser_vector.size();i++){
            if(get<CHANNEL>(muzzley_upnp_advertiser_vector[i])==channel){
              get<UPNP_ADVERTISER>(muzzley_upnp_advertiser_vector[i])->deactivate_advertiser();
              cout << "Deleted UPNP advertiser for channelid: " << channel << endl << flush;
            }
          }
          return true;
      }catch(exception& e){
          cout << "Exception: " << e.what() << endl << flush;
          return false;
      }
}

void MuzzleyUPNPManager::generate_file_muzzley_upnp_advertiser(int pos){
    try{
        MuzzleyUPNPAdvertiser* ad = get<UPNP_ADVERTISER>(muzzley_upnp_advertiser_vector[pos]);
        ad->generate_xml_file();
    }catch(exception& e){
        cout << "Exception: " << e.what() << endl << flush;
    }
}


void MuzzleyUPNPManager::loop(){
  //Run the main loop
  main_loop = g_main_loop_new (NULL, FALSE);
  g_main_loop_run (main_loop);
}

