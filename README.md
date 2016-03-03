## Muzzley-Alljoyn Gateway Connector

muzzleyconn is an application that connects local Alljoyn networks with Muzzley Cloud services over MQTT.
See http://allseenalliance.org for more information about AllJoyn, and http://muzzley.com for more information about Muzzley.

## Motivation

Normally an AllJoyn device can only communicate with other AllJoyn devices from within a local network.
This application sends AllJoyn traffic over an MQTT connection to Muzzley cloud services, and announce Muzzley compatible alljoyn devices over UPNP. This allows, for instance, a mobile phone running the Muzzley App to add those devices to the users Muzzley account, and continue to communicate with those devices even when not connected to the local AllJoyn network via WiFi.

## Building from Source

Building the source code requires first setting up dependencies and then using **scons** to produce the binaries.

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
```
sudo apt-get install libcap-dev
```

#### libxml2
```
sudo apt-get update
sudo apt-get install libxml2-dev
export LIBXML2_INCDIR = "/usr/include/libxml2"
export LIBXML2_LIBDIR = "/usr/lib"
```

#### libGlib
```
sudo apt-get install libglib2.0-dev 
export LIBGLIB_INCDIR = "/usr/include/glib-2.0"
export LIBGLIB_LIBDIR = "/usr/lib"
```

#### libSoup
```	
sudo apt-get install libsoup2.4-dev
export LIBSOUP_INCDIR = "/usr/include/libsoup-2.4"
```

#### libGupnp
```
sudo apt-get install libgupnp-1.0-dev
export LIBGUPNP_INCDIR = "/usr/include/gupnp-1.0"
```

#### libGssdp
```
sudo apt-get install libgssdp-1.0-dev
export LIBGSSDP_INCDIR = /usr/include/gssdp-1.0
```	

#### RapidJSON

It is necessary to download the RapidJSON source code (building is not necessary since the library is header-only).
Source code can be downloaded from https://github.com/miloyip/rapidjson.git.
After downloading, the RAPIDJSON\_INCDIR environment variable must be defined before building muzzleyconn.
For example, if your RapidJSON source code folder is RAPIDJSON\_ROOT, then RAPIDJSON\_INCDIR needs to point to $RAPIDJSON\_ROOT/include:
```
export RAPIDJSON_INCDIR = $RAPIDJSON_ROOT/include
```

#### Mosquitto
```
sudo apt-get install mosquitto-dev
export LIBMOSQUITTO_INCDIR = "/usr/include"
export LIBMOSQUITTO_LIBDIR = "/usr/lib"
```

#Build Instructions for x86_64

1) Follow the instructions in [here](https://allseenalliance.org/framework/documentation/develop/building/linux/build-source) to set up your ubuntu machine to build AllJoyn.

2) Create a new folder named "alljoyn-muzzley" on your home directory.

3) Use the "alljoyn-muzzley" folder as your workspace and create the following directory structure to setup your environment.

```
core/

    gwagent/   (https://git.allseenalliance.org/gerrit/gateway/gwagent.git)
     		   (git checkout RB14.12b)

	ajtcl/   (https://git.allseenalliance.org/gerrit/core/ajtcl.git)
             (git checkout RB14.12b)
	alljoyn/ (https://git.allseenalliance.org/gerrit/core/alljoyn.git)
			 (git checkout RB14.12b)

base_tcl/ (https://git.allseenalliance.org/gerrit/services/base_tcl.git)

base/ (https://git.allseenalliance.org/gerrit/services/base.git)
      (git checkout RB14.12b)

services/

       base_tcl/ (https://git.allseenalliance.org/gerrit/services/base_tcl.git)
       (git checkout RB14.12b)

       base/ (https://git.allseenalliance.org/gerrit/services/base.git)
       (git checkout RB14.12b)

```

4)  Open a command terminal and from under the **core/gwagent/cpp** directory, clone this repository.

5) Rename connetor folder for a more friendly name like "MuzzleyConnector", and open the SConscript file under the core/gwagent/cpp directory, and add the same folder name to the array **gateway_dirs**.

6) Export the following ambient variables to setup your x86/64 build environment.

```
export TARGET_OS="linux"
export TARGET_CPU="x86_64"
export TARGET="x86_64-linux-gnu"
export VARIANT=debug
export AJ_ROOT="$HOME/alljoyn-muzzley"
export GWAGENT_SRC_DIR=$AJ_ROOT/core/gwagent
export ALLJOYN_DISTDIR=$GWAGENT_SRC_DIR/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/cpp/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/config/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/controlpanel/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/notification/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/services_common/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/cpp/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/config/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/controlpanel/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/notification/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/services_common/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/services_common/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/gatewayMgmtApp/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/gatewayController/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/gatewayConnector/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/$TARGET/lib/
export PATH=$PATH:/usr/$TARGET/bin
```

7) Open a command terminal and from under the **core/alljoyn/** directory, run the command **scons** to build the core modules for x86_64 target.
```
scons V=1 OS=linux CPU=x86_64 BINDINGS="cpp" WS=off SERVICES="about,notification,controlpanel,config,onboarding,sample_apps"
```

8) Open a command terminal and from under the **core/gwagent/** directory, run the command **scons** to build the gateway agent fot x86_64 target.
```
scons V=1 OS=linux CPU=x86_64 BINDINGS="cpp" WS=off SERVICES="about,notification,controlpanel,config,onboarding,sample_apps"
```

9) If needed, run the following **scons** command to clean the build files
```
scons V=1 OS=linux CPU=x86_64 BINDINGS="cpp" WS=off SERVICES="about,notification,controlpanel,config,onboarding,sample_apps" -c
```

### Running as a normal AllJoyn application

1) When running as a normal AllJoyn application without the Gateaway Agent it is unnecessary for the gateway agent to be running.
In this case just make sure the alljoyn-daemon is running.

2) If the instructions were followed according to the above Wiki article the daemon should already exist.
It can be started as follows:
```
sudo service alljoyn start
```

3) Then the muzzleyconn application can run directly if desired.
```
./muzzleyconn
```

4) Next set up the configuration file:
In the terminal navigate to the **/etc/muzzleyconn** folder and then open the **muzzleyconn\_factory.conf** file as superuser to edit it.
```
sudo gedit /etc/muzzleyconn/muzzleyconn_factory.conf
```

5) These arguments can be optionally modified as needed:
```
Verbosity - level of debug output verbosity. Can be 0, 1, or 2, with 2 being the most verbose
```
    
The file looks like the following:
```
{
    "ProductID": "muzzleyconn",
    "SerialNumber": "muzzleyconn01",
    "DeviceName" : "MuzzleyConnector",
    "AppName" : "Muzzley MQTT Connector",
    "Manufacturer" : "Muzzley",
    "ModelNumber" : "MuzzleyConnector",
    "Description" : "Muzzley Alljoyn MQTT connector",
    "DateOfManufacture" : "2016-03-01",
    "SoftwareVersion" : "0.0.1",
    "HardwareVersion" : "0.0.1",
    "SupportUrl" : "http://www.muzzley.com",
    "Verbosity":"2",
    "Compress":"1"
}
```

6) Save and close the file. Now copy that file to /etc/muzzleyconn/muzzleyconn.conf as follows:
```
sudo cp /etc/muzzleyconn/muzzleyconn_factory.conf /etc/muzzleyconn/muzzleyconn.conf
``` 

7) You are now ready to connect the muzzleyconn connector to the Muzzley cloud.

#Build Instructions for OpenWRT

#### Prepare the OpenWRT Toolchain 

1) Download the OpenWRT source code from: 
```
git clone git://git.openwrt.org/14.07/openwrt.git
```

2) Download the config file for the correspondent hardware where the OpenWRT will be run from [here](https://downloads.openwrt.org) and paste it on the root of the openwrt project.

3) Copy downloaded config file to .config (for example):
```
cp config.ar71xx_generic .config
```

4) Copy feeds.conf.default to feeds.conf (if not already done):
```
cp feeds.conf.default feeds.conf
```

5) Add the following line to the end of the file:
```
src-git alljoyn https://git.allseenalliance.org/gerrit/core/openwrt_feed;barrier_breaker

```

6) Update the feed information:
```
./scripts/feeds update -a
```
    
7) Add the the packages from the feeds to build system (luci interface is not needed but recommended to configure the router using a web interface):
```
./scripts/feeds install -a -p alljoyn
./scripts/feeds install libgupnp
./scripts/feeds install libgssdp
./scripts/feeds install -a -p luci
```

8) Enable AllJoyn in the build:
```
make menuconfig
```

```
     Networking --->
          < > alljoyn --->
               < > alljoyn-about
               < > alljoyn-c
               < > alljoyn-config
                    < > alljoyn-config-samples
               < > alljoyn-controlpanel
                    < > alljoyn-controlpanel-samples
               < > alljoyn-notification
                    < > alljoyn-notification-samples
               < > alljoyn-onboarding
                    < > alljoyn-onboarding-samples
               < > alljoyn-sample_apps
               < > alljoyn-samples
               < > alljoyn-service_common
     LuCI --->
          < > Collections --->
               < > luci     
          < > Themes --->
               < > luci-themes-openwrt
     Libraries --->
          < > libxml2
          < > libgupnp
          < > libgssdp
          < > mosquitto
          
```

9) Make the firmware image including the correspondent configuration using the command:
```
make
```

10) Flash it in the router firmware and wait for reboot.


#### muzzleyconn

1) Export the following ambient variables to setup your openwrt build environment.

```
export TARGET_OS="openwrt"
export TARGET_CPU="openwrt"
export TARGET=mips-openwrt-linux-uclibc
export VARIANT=debug
export AJ_ROOT="$HOME/alljoyn-muzzley"
export AJ_DIST="$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/debug/dist"
export GWAGENT_SRC_DIR=$AJ_ROOT/core/gwagent
export ALLJOYN_DISTDIR=$GWAGENT_SRC_DIR/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist
export OPENWRT_BASE="$HOME/openwrt"
export OPENWRT_TOOLCHAIN_BASE=$OPENWRT_BASE/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/
export OPENWRT_TARGET_BASE=$OPENWRT_BASE/staging_dir/target-mips_34kc_uClibc-0.9.33.2/
export STAGING_DIR=$OPENWRT_BASE/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2
export PATH=$PATH:$OPENWRT_BASE/staging_dir/toolchain-mips_34kc_gcc-4.8-linaro_uClibc-0.9.33.2/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/cpp/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/config/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/controlpanel/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/notification/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/alljoyn/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/services_common/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/cpp/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/config/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/controlpanel/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/notification/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/services_common/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/services_common/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/gatewayMgmtApp/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/gatewayController/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$AJ_ROOT/core/gwagent/build/$TARGET_OS/$TARGET_CPU/$VARIANT/dist/gatewayConnector/lib/
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:/usr/$TARGET/lib/
export PATH=$PATH:/usr/$TARGET/bin
```

**NOTE:** Before building, make sure that **TARGET**, **RAPIDJSON\_INCDIR**, **LIBGLIB_INCDIR**, **LIBGLIB_LIBDIR**, **MOSQUITTO_INCDIR** and **MOSQUITTO_LIBDIR** are set appropriately, accordingly with your hardware architecture.
For ex.: **mips-openwrt-linux-uclibc**


### Running as an AllJoyn Gateway Connector application

The following commands are targgted for a **mips-openwrt-linux-uclibc** hardware architecture.
Some command changes may by needed for a different architecture.

#### Compiling the connector

1) To compile the connector open a new terminal window under **~/alljoyn-muzzley/core/gwagent/**, and run the following **scons** compile command:
```
scons V=1 ICE=off BR=on BT=off WS=off CPU=openwrt OS=openwrt BINDINGS="cpp" SERVICES="about,notification,controlpanel,config,onboarding,sample_apps" TARGET_CFLAGS="-Os -pipe -mips32r2 -mtune=74kc -fPIC -fno-caller-saves -fhonour-copts -Wno-error=unused-but-set-variable -msoft-float" "TARGET_CC=$TARGET-gcc" "TARGET_CXX=$TARGET-g++" "TARGET_AR=$TARGET-ar" "TARGET_RANLIB=$TARGET-ranlib" "TARGET_LINK=$TARGET-gcc" "TARGET_CPPFLAGS=-I$OPENWRT_TARGET_BASE/usr/include -I$OPENWRT_TARGET_BASE/include -I$OPENWRT_TOOLCHAIN_BASE/usr/include -I$OPENWRT_TOOLCHAIN_BASE/include" "TARGET_PATH=$OPENWRT_TOOLCHAIN_BASE/bin:$OPENWRT_BASE/staging_dir/host/bin:$PATH" "STAGING_DIR=$OPENWRT_TARGET_BASE" "TARGET_LINKFLAGS=-L$OPENWRT_TARGET_BASE/usr/lib" "CXXFLAGS=$CXXFLAGS -I$AJ_DIST/cpp/inc -I$AJ_DIST/about/inc -I$AJ_DIST/services_common/inc -I$AJ_DIST/notification/inc -I$AJ_DIST/controlpanel/inc -I$AJ_DIST/services_common/inc" "LDFLAGS=$LDFLAGS -L$AJ_DIST/cpp/lib -L$AJ_DIST/about/lib -L$AJ_DIST/services_common/lib -L$AJ_DIST/notification/lib -L$AJ_DIST/controlpanel/lib"
```

2) To clean the build environment, just add a "-c" at the end of the previous command.
```
scons V=1 ICE=off BR=on BT=off WS=off CPU=openwrt OS=openwrt BINDINGS="cpp" SERVICES="about,notification,controlpanel,config,onboarding,sample_apps" TARGET_CFLAGS="-Os -pipe -mips32r2 -mtune=74kc -fPIC -fno-caller-saves -fhonour-copts -Wno-error=unused-but-set-variable -msoft-float" "TARGET_CC=$TARGET-gcc" "TARGET_CXX=$TARGET-g++" "TARGET_AR=$TARGET-ar" "TARGET_RANLIB=$TARGET-ranlib" "TARGET_LINK=$TARGET-gcc" "TARGET_CPPFLAGS=-I$OPENWRT_TARGET_BASE/usr/include -I$OPENWRT_TARGET_BASE/include -I$OPENWRT_TOOLCHAIN_BASE/usr/include -I$OPENWRT_TOOLCHAIN_BASE/include" "TARGET_PATH=$OPENWRT_TOOLCHAIN_BASE/bin:$OPENWRT_BASE/staging_dir/host/bin:$PATH" "STAGING_DIR=$OPENWRT_TARGET_BASE" "TARGET_LINKFLAGS=-L$OPENWRT_TARGET_BASE/usr/lib" "CXXFLAGS=$CXXFLAGS -I$AJ_DIST/cpp/inc -I$AJ_DIST/about/inc -I$AJ_DIST/services_common/inc -I$AJ_DIST/notification/inc -I$AJ_DIST/controlpanel/inc -I$AJ_DIST/services_common/inc" "LDFLAGS=$LDFLAGS -L$AJ_DIST/cpp/lib -L$AJ_DIST/about/lib -L$AJ_DIST/services_common/lib -L$AJ_DIST/notification/lib -L$AJ_DIST/controlpanel/lib" -c
```

#### Installation

**Note:** An AllJoyn daemon must be running on the same local system for this program to work. Refer to http://wiki.allseenalliance.org/gateway/getting\_started to learn how to set up your system with the proper AllJoyn dependencies.

1) You need to create a directory structure for the muzzleyconn app:
```
sudo mkdir -p /opt/alljoyn/apps/muzzleyconn/acls
sudo mkdir -p /opt/alljoyn/apps/muzzleyconn/bin
sudo mkdir -p /opt/alljoyn/apps/muzzleyconn/lib
sudo mkdir -p /opt/alljoyn/apps/muzzleyconn/store
sudo mkdir -p /opt/alljoyn/apps/muzzleyconn/etc
```

2) Under Gateway Connector, the muzzleyconn process will be run as "muzzleyconn" user. It needs to be able to write to the "etc" subdirectory. Since we created the directory structure above as root (sudo), change the owner and the group of that directory:
```
sudo chown -R muzzleyconn /opt/alljoyn/apps/muzzleyconn
sudo chgrp -R muzzleyconn /opt/alljoyn/apps/muzzleyconn
```

3) Copy the resulting executable, to the "bin" subdirectory of muzzleyconn app:
```
sudo cp $ROOTPATH/muzzleyconn/build/muzzleyconn /opt/alljoyn/apps/muzzleyconn/bin
```

4) Copy the Manifest file to the top-level muzzleyconn app directory:
```
sudo cp $ROOTPATH/muzzleyconn/Manifest.xml /opt/alljoyn/apps/muzzleyconn
```

5) The Manifest file has to be modified to allow the muzzleyconn process to be run as "muzzleyconn" user. Add the following line after the <env_variables> line:
```
<variable name="HOME">/home/muzzleyconn</variable>
```

6) The Gateway Connector app needs a configuration file. Copy your muzzleyconn_factory.conf file to: **/opt/alljoyn/apps/muzzleyconn/etc/muzzleyconn_factory.conf**.
Note that the "store" and "acls" subdirectories will remain empty for now. You are now ready to execute muzzleyconn as a Gateway Connector app.

#### Running the MUZZLEYCONN with the Gateway Agent (Working on it..)

##### On the x86_64 target

1) Start the Gateway Agent:
```
sudo service alljoyn-gwagent start
``` 

2) Verify that it is running:
```
sudo service alljoyn-gwagent status
``` 

##### On the OpenWRT target

1) Start the Gateway Agent:
```
alljoyn-gwagent
``` 

2) Verify that it is running:
```
ps -ef | grep alljoyn-gateway
``` 

3) The instructions for downloading and running the Gateway Connector Appplication on the Android are on the AllSeen Alliance website.
After installing the app, open it and click on AllJoyn Gateway Configuration Manager.
You should see "Alljoyn Muzzley Connector" in the Gateway Connector Applications list.
At this point, the state of the app should show "Stopped".
This is because we haven't created any Access Control Lists (ACL's) yet.

#### Creating an ACL

1) Click on the "Muzzley" button to open the MQTT Connector app. Using the context menu on your Android device, click on "Create ACL".
This will open up a window where you choose a name for your ACL, and choose which services will be allowed to pass through muzzleyconn.
For now, select the "Expose all services" checkbox, since we want to ensure that the muzzleyconn app works just as the command-line muzzleyconn.
Click on "Create".


2) Go back to the previous window (the Muzzley Connector app). You will see that it still shows up as "Stopped".
First, you need to make sure that the newly created ACL is in the "Active" state.
Then, from the Linux command line, restart the Gateway Agent, and verify the it is running.

NOTE: It is possible that the Gateway Agent is not running at this point (you might see the message "The process appears to be dead but pidfile still exists"). 
If this happens, you will need to restart the AllJoyn service, and then restart the Gateway Agent:

4) The Gateway Agent should now be running.


#### Running the MUZZLEYCONN as standalone

1) Start the Muzzley Connector, simply running it:
```
cd /opt/alljoyn/apps/muzzleyconn/bin
./muzzleyconn
```

2) Check the logs for any error.


## License

Copyright (c) 2016, Muzzley

Copyright (c) 2015, Affinegy, Inc.

Permission to use, copy, modify, and/or distribute this software for any purpose with or without fee is hereby granted, provided that the above copyright notice and this permission notice appear in all copies.

THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
