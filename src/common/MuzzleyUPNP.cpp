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


#include "MuzzleyUPNP.h"

using namespace std;

MuzzleyUPNPAdvertiser::MuzzleyUPNPAdvertiser(string interface, string host, int port, string xml_fileName, string xml_filePath,
								string profileId, string friendlyName, string channelId, string macAddress,
								string serialNumber, string manufacturer, string manufacturer_Url, string modelName,
								string modelNumber, string modelDescription, vector <component> modelComponents){

  device_xml_fileName = xml_fileName;
  device_xml_filePath = xml_filePath;
  device_interface = interface;
  device_host = host;
  device_port = port;

  device_profileId = profileId;
  device_udn = profileId;
  device_channelId = channelId;
  device_friendlyName = friendlyName;
  device_macAddress = macAddress;
  device_serialNumber = serialNumber;

  device_manufacturer = manufacturer;
  device_manufacturer_url = manufacturer_Url;
  device_modelName = modelName;
  device_modelNumber = modelNumber;
  device_modelDescription = modelDescription;
  components = modelComponents;

  error = NULL;
  
  #if !GLIB_CHECK_VERSION(2,35,0)
     g_type_init ();
  #endif

  generate_xml_file();

  gupnp_context = gupnp_context_new (NULL, device_interface.c_str(), device_port, &error);
  if (error) {
    g_printerr ("Error creating the GUPnP context: %s\n", error->message);
    g_error_free (error);
  }

  gupnp_root_dev = gupnp_root_device_new (gupnp_context, device_xml_fileName.c_str(), device_xml_filePath.c_str());
  gupnp_root_device_set_available (gupnp_root_dev, TRUE);
  gupnp_resource_group = gupnp_root_device_get_ssdp_resource_group(gupnp_root_dev);
  gssdp_resource_group_set_max_age (gupnp_resource_group, GUPNP_MAX_AGE);
  gssdp_resource_group_set_message_delay(gupnp_resource_group, GUPNP_MESSAGE_DELAY);

  std::stringstream stream_urn;
  stream_urn << "urn:Muzzley:device:" << device_profileId << ":1";
  device_urn = stream_urn.str();

  std::stringstream stream_url;
  stream_url << "http://" << gupnp_context_get_host_ip (gupnp_context) << ":" << gupnp_context_get_port (gupnp_context) << "/" << gupnp_root_device_get_relative_location(gupnp_root_dev);
  device_url = stream_url.str(); 

  device_port=gupnp_context_get_port(gupnp_context);
  device_xml_descriptionPath=gupnp_root_device_get_description_path(gupnp_root_dev);

 
}

MuzzleyUPNPAdvertiser::~MuzzleyUPNPAdvertiser(){
}

void MuzzleyUPNPAdvertiser::get_info(){

  cout << "URN: " << device_urn << endl << flush;
  cout << "HOST: " << device_host << endl << flush;
  cout << "PORT: " << device_port << endl << flush;
  cout << "URL: " << device_url << endl << flush;
  cout << "XML FILE PATH: " << device_xml_filePath << endl << flush;
  cout << "XML FILE NAME: " << device_xml_fileName << endl << flush;
  cout << "INTERFACE: " << device_interface << endl << flush;
  cout << "FRIENDLY NAME: " << device_friendlyName << endl << flush;
  cout << "UDN: " << device_udn << endl << flush;
  cout << "SERIAL NUMBER: " << device_serialNumber << endl << flush;
  cout << "MAC ADDRESS: " << device_macAddress << endl << flush;
  cout << "MANUFACTURER: " << device_manufacturer << endl << flush;
  cout << "MANUFACTURER URL: " << device_manufacturer_url << endl << flush;
  cout << "MODEL NAME: " << device_modelName << endl << flush;
  cout << "MODEL NUMBER: " << device_modelNumber << endl << flush;
  cout << "MODEL DESCRIPTION: " << device_modelDescription << endl << endl << flush;

}


void MuzzleyUPNPAdvertiser::generate_xml_file(){

  std::stringstream responseStream;

  responseStream << "<?xml version=\"1.0\"?>\n";
  responseStream << "<root>\n";
  responseStream << "<specVersion>\n";
  responseStream << "<major>1</major>\n";
  responseStream << "<minor>0</minor>\n";
  responseStream << "</specVersion>\n";
  responseStream << "<device>\n";
  responseStream << "<deviceType>urn:Muzzley:device:" << device_profileId << ":1</deviceType>\n";
  responseStream << "<friendlyName>" << device_friendlyName << "</friendlyName>\n";
  responseStream << "<manufacturer>" << device_manufacturer << "</manufacturer>\n";
  responseStream << "<manufacturerURL>" << device_manufacturer_url << "</manufacturerURL>\n";
  responseStream << "<modelDescription>" << device_modelDescription << "</modelDescription>\n";
  responseStream << "<modelName>" << device_modelName << "</modelName>\n";
  responseStream << "<modelNumber>" << device_modelNumber << "</modelNumber>\n";
  responseStream << "<UDN>uuid:" << device_udn << "</UDN>\n";
  responseStream << "<serialNumber>" << device_serialNumber << "</serialNumber>\n";
  responseStream << "<macAddress>" << device_macAddress << "</macAddress>\n";
  responseStream << "<deviceKey>" << device_channelId << "</deviceKey>\n";
  responseStream << "<components>\n";
  try{
    for (unsigned int i = 0; i < components.size(); i++){
        if(get<0>(components[i])!=""){
            responseStream << "<component>\n";
            responseStream << "<id>" << get<0>(components[i]) << "</id>\n";
            responseStream << "<label>" << get<1>(components[i]) << "</label>\n";
            responseStream << "<type>" << get<2>(components[i]) << "</type>\n";
            responseStream << "</component>\n";
        }
    }
    
  }catch(exception& e){
      cout << "Exception: " << e.what() << endl << flush;
  }
  responseStream << "</components>\n";
  responseStream << "</device>\n";
  responseStream << "</root>\n";


  ofstream myfile;
  myfile.open (device_xml_filePath + "/" + device_xml_fileName);
  const std::string tmp = responseStream.str();
  const char* str_xml = tmp.c_str();
  myfile << str_xml;
  myfile.close();
}

void MuzzleyUPNPAdvertiser::update_xml_file(string interface, string host, int port, string xml_fileName, string xml_filePath,
                string profileId, string friendlyName, string channelId, string macAddress,
                string serialNumber, string manufacturer, string manufacturer_Url, string modelName,
                string modelNumber, string modelDescription, vector <component> modelComponents){

  device_xml_fileName = xml_fileName;
  device_xml_filePath = xml_filePath;
  device_interface = interface;
  device_host = host;
  device_port = port;

  device_profileId = profileId;
  device_udn = profileId;
  device_channelId = channelId;
  device_friendlyName = friendlyName;
  device_macAddress = macAddress;
  device_serialNumber = serialNumber;

  device_manufacturer = manufacturer;
  device_manufacturer_url = manufacturer_Url;
  device_modelName = modelName;
  device_modelNumber = modelNumber;
  device_modelDescription = modelDescription;
  components = modelComponents;

  generate_xml_file();

}


void MuzzleyUPNPAdvertiser::deactivate_advertiser(){
  gupnp_root_device_set_available(gupnp_root_dev, FALSE);
}

void MuzzleyUPNPAdvertiser::activate_advertiser(){
  gupnp_root_device_set_available(gupnp_root_dev, TRUE);
}