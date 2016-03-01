## Synopsis

muzzleyconn is an application that connects local Alljoyn networks with Muzzley Cloud services over MQTT. See http://allseenalliance.org for more information about AllJoyn, and http://muzzley.com for more information about Muzzley.

## Motivation

Normally an AllJoyn device can only communicate with other AllJoyn devices from within a local network. This application sends AllJoyn traffic over an MQTT connection to Muzzley cloud services, and announce Muzzley compatible alljoyn devices over UPNP. This allows, for instance, a mobile phone running the Muzzley App to add those devices to the users Muzzley account, and continue to communicate with those devices even when not connected to the local AllJoyn network via WiFi.

## Building from Source

Building the source code requires first setting up dependencies and then using scons to produce the binaries.

### Dependencies

The following dependencies must be obtained and installed:

* libcap-dev
* libsoup-dev
* libxml2-dev
* libgupnp-1.0-dev
* libgssdp-1.0-dev
* libglib2.0-dev
* libssl-dev
* libcrypto-dev
* mosquitto-dev
* RapidJSON
* AllJoyn Gateway Agent
* Alljoyn Core


#### libcap


#### libxml2

    sudo apt-get update
    sudo apt-get install libxml2-dev

    export LIBXML2_INCDIR = "/usr/include/libxml2"
	export LIBXML2_LIBDIR = "/usr/lib"

#### libGlib
 	
 	export LIBGLIB_INCDIR = "/usr/include/glib-2.0"
	export LIBGLIB_LIBDIR = "/usr/mips-openwrt-linux-uclibc/lib"

#### libSoup
 	
 	export LIBSOUP_INCDIR = "/usr/include/libsoup-2.4"

#### libGupnp
 	
 	export LIBGUPNP_INCDIR = "/usr/include/gupnp-1.0"
	
#### libGssdp
 	
 	export LIBGSSDP_INCDIR = /usr/include/gssdp-1.0
	

#### RapidJSON

It is necessary to download the RapidJSON source code (building is not necessary since the library is header-only). Source code can be downloaded from https://github.com/miloyip/rapidjson.git. After downloading, the RAPIDJSON\_INCDIR environment variable must be defined before building muzzleyconn. For example, if your RapidJSON source code folder is RAPIDJSON\_ROOT, then RAPIDJSON\_INCDIR needs to point to $RAPIDJSON\_ROOT/include:

    export RAPIDJSON_INCDIR = $RAPIDJSON_ROOT/include
    
#### Mosquitto

	export LIBMOSQUITTO_INCDIR = "/usr/mips-openwrt-linux-uclibc/include"
	export LIBMOSQUITTO_LIBDIR = "/usr/mips-openwrt-linux-uclibc/lib"
	

#### AllJoyn Gateway Agent

Pull the source code and build the AllJoyn Gateway Agent as follows. 

    cd $ROOTPATH
    mkdir -p alljoyn_src/core alljoyn_src/services alljoyn_src/gateway
    cd alljoyn_src/core
    git clone https://git.allseenalliance.org/gerrit/core/alljoyn
    cd alljoyn
    git checkout RB14.12b
    cd $ROOTPATH/alljoyn_src/services
    git clone https://git.allseenalliance.org/gerrit/services/base
    cd base
    git checkout RB14.12
    cd $ROOTPATH/alljoyn_src/gateway
    git clone https://git.allseenalliance.org/gerrit/gateway/gwagent
    cd gwagent
    git checkout RB14.12
    export GWAGENT_SRC_DIR=`pwd`
    unset ALLJOYN_DISTDIR
    export VARIANT=debug
    scons V=1 OS=$OS CPU=$CPU BINDINGS=cpp VARIANT=$VARIANT WS=off POLICYDB=on
    export ALLJOYN_DISTDIR=$GWAGENT_SRC_DIR/build/linux/$CPU/$VARIANT/dist
    mkdir -p $ROOTPATH/build/lib
    find $ALLJOYN_DISTDIR -name "*\.so" -exec cp {} $ROOTPATH/build/lib/ \;
    export LD_LIBRARY_PATH=$ROOTPATH/build/lib

**NOTE:** If the scons command fails then refer to http://wiki.allseenalliance.org/gateway/getting\_started for more information.

#### muzzleyconn

**NOTE:** Before building, make sure that RAPIDJSON\_PATH and ALLJOYN\_DISTDIR environment variables, described above, are set appropriately.

Pull the source code from the repository into the muzzleyconn folder under $ROOTPATH, and run "make", specifying that we are NOT building a Gateway Connector app (explained in the next section):

    cd $ROOTPATH
    git clone https://bitbucket.org/jorgeclaro/muzzleyconn.git
    cd muzzleyconn
    make NO_AJ_GATEWAY=1

## Installation

An AllJoyn daemon must be running on the same local system for this program to work. Refer to http://wiki.allseenalliance.org/gateway/getting\_started to learn how to set up your system with the proper AllJoyn dependencies.

Once that is completed it is possible to install this application in two ways:

1. As a normal AllJoyn application
2. As an AllJoyn Gateway Connector application

### Running as a normal AllJoyn application

When running as a normal AllJoyn application without the Gateaway Agent it is unnecessary for the gateway agent to be running. In this case just make sure the alljoyn-daemon is running. If the instructions were followed according to the above Wiki article the daemon should already exist. It can be started as follows:

    sudo service alljoyn start

Then the muzzleyconn application can run directly if desired:

    cd $ROOTPATH/muzzleyconn
    build/muzzleyconn

If it is desired that muzzleyconn be installed do the following:

    cd $ROOTPATH/muzzleyconn
    sudo cp build/muzzleyconn /usr/bin/
    sudo cp conf/muzzleyconn.init /etc/init.d/muzzleyconn
    cd /etc/rc3.d
    sudo ln -s ../init.d/muzzleyconn S95muzzleyconn

Next set up the configuration file:

In the terminal navigate to the /etc/muzzleyconn folder and then open the muzzleyconn\_factory.conf file as superuser to edit it.

    sudo gedit /etc/muzzleyconn/muzzleyconn_factory.conf

You will need to get the ProductID from the developer portal (it will be assigned when you create a new product).
Paste the ProductID into the ProductID field in the muzzleyconn\_factory.conf file that you just opened.

You also need a SerialNumber for your product. You can type any alphanumeric string in that field for now. But be aware that it must be a unique serial number. When you try to register more than one device with the same serial number the server will return an error. This will happen during the pairing sequence that we discuss later.

These arguments can be optionally modified as needed:

    Verbosity - level of debug output verbosity. Can be 0, 1, or 2, with 2 being the most verbose
    Compress - whether or not to compress the body of each message. This is recommended, and must match what the paired device is doing.

The file looks like the following:

    {
     "ProductID": "Your Product ID",
     "SerialNumber": "Your Serial Number",
     "DeviceName" : "My Device Name",
     "AppName" : "AllJoyn MQTT Connector",
     "Manufacturer" : "My Manufacturer Name",
     "ModelNumber" : "My Model Number",
     "Description" : "Description of my device",
     "DateOfManufacture" : "1970-01-01",
     "SoftwareVersion" : "0.0.1",
     "HardwareVersion" : "0.0.1",
     "SupportUrl" : "http://www.example.org",
     "Verbosity":"2",
     "Compress":"1"
    }

Save and close the file. Now copy that file to /etc/muzzleyconn/muzzleyconn.conf as follows:
     
    sudo cp /etc/muzzleyconn/muzzleyconn_factory.conf /etc/muzzleyconn/muzzleyconn.conf
    
You are now ready to connect the muzzleyconn service with the mobile app.

Start the MQTT connector by typing *sudo muzzleyconn* to see if the file is valid. If the file isn't valid, the terminal will tell you that the muzzlwyconn.conf file is not valid. If it is running without error you can stop it by pressing Ctrl+C.
    

### Running as an AllJoyn Gateway Connector application

The previous section described how to run muzzleyconn as a service via the Linux command line. You can also run it as a Gateway Connector phone app. The functionality should be the same in both cases.

#### Installation

You need to create a directory structure for the muzzleyconn app:

    sudo mkdir -p /opt/alljoyn/apps/muzzleyconn/acls
    sudo mkdir -p /opt/alljoyn/apps/muzzleyconn/bin
    sudo mkdir -p /opt/alljoyn/apps/muzzleyconn/lib
    sudo mkdir -p /opt/alljoyn/apps/muzzleyconn/store
    sudo mkdir -p /opt/alljoyn/apps/muzzleyconn/etc

Under Gateway Connector, the muzzleyconn process will be run as "muzzleyconn" user. It needs to be able to write to the "etc" subdirectory. Since we created the directory structure above as root (sudo), change the owner and the group of that directory:

     sudo chown -R muzzleyconn /opt/alljoyn/apps/muzzleyconn
     sudo chgrp -R muzzleyconn /opt/alljoyn/apps/muzzleyconn

Note that in the previous section, we ran the command "make NO\_AJ\_GATEWAY=1" in the $ROOTPATH/muzzleyconn directory. The NO\_AJ\_GATEWAY flag means that we are building the "standalone" version of MQTT Connector. This time, we will build it without that flag:

    cd $ROOTPATH/muzzleyconn
    make

Copy the resulting executable, to the "bin" subdirectory of muzzleyconn app:

    sudo cp $ROOTPATH/muzzleyconn/build/muzzleyconn /opt/alljoyn/apps/muzzleyconn/bin

Copy the Manifest file to the top-level muzzleyconn app directory:

    sudo cp $ROOTPATH/muzzleyconn/Manifest.xml /opt/alljoyn/apps/muzzleyconn

The Manifest file has to be modified to allow the muzzleyconn process to be run as "muzzleyconn" user. Add the following line after the <env_variables> line:

    <variable name="HOME">/home/muzzleyconn</variable>

Just like the standalone muzzleyconn, the Gateway Connector app needs a configuration file. Its format is the same as previously described, but you will need to place it in a different location. Copy your muzzleyconn_factory.conf file from the previous section to:

    /opt/alljoyn/apps/muzzleyconn/etc/muzzleyconn_factory.conf

Note that the "store" and "acls" subdirectories will remain empty for now. You are now ready to execute muzzleyconn as a Gateway Connector app.

#### Running the Gateway Connector

Start the Gateway Agent:

    sudo service alljoyn-gwagent start
    
Verify that it is running:

    sudo service alljoyn-gwagent status
    
The instructions for downloading and running the Gateway Connector app are on the AllSeen Alliance website at ["Installing the Gateway Controller Sample Android App"](https://wiki.allseenalliance.org/gateway/getting\_started#installing\_the\_gateway\_controller\_sample\_android\_app). After installing the app, open it and click on AllJoyn Gateway Configuration Manager. You should see "Muzzley Connector" (a button that says Affin...) in the Gateway Connector Applications list. At this point, the state of the app should show "Stopped". This is because we haven't created any Access Control Lists (ACL's) yet.

#### Creating an ACL

Click on the "Affin..." button to open the MQTT Connector app. Using the context menu on your Android device, click on "Create ACL". This will open up a window where you choose a name for your ACL, and choose which services will be allowed to pass through muzzleyconn. For now, select the "Expose all services" checkbox, since we want to ensure that the xmpconn app works just as the command-line muzzleyconn. Click on "Create".

Go back to the previous window (the Muzzley Connector app). You will see that it still shows up as "Stopped". First, you need to make sure that the newly created ACL is in the "Active" state. Then, from the Linux command line, restart the Gateway Agent:

    sudo service alljoyn-gwagent restart
    
Verify that it is running:

    sudo service alljoyn-gwagent status
    
NOTE: It is possible that the Gateway Agent is not running at this point (you might see the message "The process appears to be dead but pidfile still exists"). If this happens, you will need to restart the AllJoyn service, and then restart the Gateway Agent:

    sudo service alljoyn restart
    sudo service alljoyn-gwagent restart
    
The Gateway Agent should now be running.


#### Verifying the MQTT connector

In the Gateway Connector app, you should now see the MQTT connector status as "Running". On your Linux system, you should also be able to see it from the process list:

    $ ps -ef | grep muzzleyconn
    muzzleyconn 26639 26502 16 17:03 ? 00:00:12 [muzzleyconn]
    alljoyn 29748 9804 0 17:04 pts/0 00:00:00 grep --color=auto muzzleyconn
    

## License

Copyright (c) 2015, Affinegy, Inc.

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
