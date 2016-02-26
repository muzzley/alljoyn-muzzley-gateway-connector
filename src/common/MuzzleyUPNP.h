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

#include <libgupnp/gupnp.h>
#include <libgssdp/gssdp.h>

#include <vector>
#include <tuple>

#include <iostream>
#include <fstream>
#include <cstring>
#include <sstream>

using namespace std;

#define GUPNP_MAX_AGE 1800
#define GUPNP_MESSAGE_DELAY 120

typedef tuple <string, string, string> component;

class MuzzleyUPNPAdvertiser {
  public:

    MuzzleyUPNPAdvertiser(string interface, string host, int port, string xml_fileName, string xml_filePath,
      string profileId, string friendlyName, string channelId, string macAddress, string serialNumber, 
      string manufacturer, string manufacturer_Url, string modelName, string modelNumber, 
      string modelDescription, vector<component> modelComponents);

    ~MuzzleyUPNPAdvertiser();

    void get_info();
    void generate_xml_file();
    void update_xml_file(string interface, string host, int port, string xml_fileName, string xml_filePath,
      string profileId, string friendlyName, string channelId, string macAddress, string serialNumber, 
      string manufacturer, string manufacturer_Url, string modelName, string modelNumber, 
      string modelDescription, vector<component> modelComponents);
    void deactivate_advertiser();
    void activate_advertiser(); 
  private:

    GError *error;
    GUPnPContext *gupnp_context;
    GUPnPRootDevice *gupnp_root_dev;
    GSSDPResourceGroup *gupnp_resource_group;
  
    string device_xml_descriptionPath;
    string device_interface;
    string device_host;
    int device_port;

    string device_urn;
    string device_udn;
    
    string device_xml_fileName;
    string device_xml_filePath;
    string device_url;
    string device_profileId;
    string device_friendlyName;
    string device_channelId;
    string device_macAddress;
    string device_serialNumber;
    string device_manufacturer;
    string device_manufacturer_url;
    string device_modelName;
    string device_modelNumber;
    string device_modelDescription;
    vector <component> components;

};

