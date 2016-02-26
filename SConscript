# Copyright (c) 2014, AllSeen Alliance. All rights reserved.
#
#    Permission to use, copy, modify, and/or distribute this software for any
#    purpose with or without fee is hereby granted, provided that the above
#    copyright notice and this permission notice appear in all copies.
#
#    THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
#    WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
#    MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
#    ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
#    WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
#    ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
#    OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.

import os

Import('gateway_env')

vars = Variables()

vars.Add('BINDINGS', 'Bindings to build (comma separated list): cpp', 'cpp')

vars.Add(PathVariable('ALLJOYN_DISTDIR',
                      'Directory containing a pre-built AllJoyn Core dist directory.',
                      os.environ.get('ALLJOYN_DISTDIR')))

vars.Add(PathVariable('LIBXML2_INCDIR',
                      'Directory containing the libxml2 include files.',
                      os.environ.get('LIBXML2_INCDIR','/usr/include/libxml2')))

vars.Add(PathVariable('LIBXML2_LIBDIR',
                      'Directory containing the libxml2 library files.',
                      os.environ.get('LIBXML2_LIBDIR','/usr/lib')))

vars.Add(PathVariable('RAPIDJSON_INCDIR',
                      'Directory containing the rapidjson include folder.',
                      os.environ.get('RAPIDJSON_INCDIR','/usr/include')))

vars.Add(PathVariable('LIBMOSQUITTO_INCDIR',
                      'Directory containing the mosquitto include files.',
                       os.environ.get('LIBMOSQUITTO_INCDIR','/usr/mips-openwrt-linux-uclibc/include')))

vars.Add(PathVariable('LIBMOSQUITTO_LIBDIR',
                      'Directory containing the mosquitto library files.',
                      os.environ.get('LIBMOSQUITTO_LIBDIR','/usr/mips-openwrt-linux-uclibc/lib')))

vars.Add(PathVariable('LIBGLIB_INCDIR',
                      'Directory containing the glib include files.',
                       os.environ.get('LIBGLIB_INCDIR','/usr/include/glib-2.0')))

vars.Add(PathVariable('LIBGLIB_LIBDIR',
                      'Directory containing the glib library files.',
                      os.environ.get('LIBGLIB_LIBDIR','/usr/mips-openwrt-linux-uclibc/lib')))

vars.Add(PathVariable('LIBSOUP_INCDIR',
                      'Directory containing the soup include files.',
                       os.environ.get('LIBSOUP_INCDIR','/usr/include/libsoup-2.4')))

vars.Add(PathVariable('LIBGUPNP_INCDIR',
                      'Directory containing the gupnp include files.',
                       os.environ.get('LIBGUPNP_INCDIR','/usr/include/gupnp-1.0')))

vars.Add(PathVariable('LIBGSSDP_INCDIR',
                      'Directory containing the gssdp include files.',
                       os.environ.get('LIBGSSDP_INCDIR', '/usr/include/gssdp-1.0')))

vars.Add(EnumVariable('USE_GATEWAY_AGENT',
                      'Build with Gateway Agent as a dependency.',
                      'off',
                      allowed_values = ['off', 'on']))

vars.Add(EnumVariable('FULLCLEAN',
                      'Used with the scons -c option to clean everything, including AllJoyn dependencies.',
                      'off',
                      allowed_values = ['off', 'on']))

vars.Update(gateway_env)

gateway_env['MUZZLEY_DISTDIR'] = gateway_env['DISTDIR'] + '/MuzzleyConnector'

gateway_env.Append(LIBPATH = '$MUZZLEY_DISTDIR/lib');
gateway_env.Append(CPPPATH = '$MUZZLEY_DISTDIR/inc');


# Set the flag True for standalone connector executable
gateway_env.Append(NO_AJ_GATEWAY = False)


gateway_env.Install('$MUZZLEY_DISTDIR/inc/transport', gateway_env.Glob('src/transport/*.h'))
gateway_env.Install('$MUZZLEY_DISTDIR/inc/common', gateway_env.Glob('src/common/*.h'))
gateway_env.Install('$MUZZLEY_DISTDIR/inc/app', gateway_env.Glob('src/app/*.h'))
gateway_env.Install('$MUZZLEY_DISTDIR/conf', gateway_env.Glob('conf/muzzleyconn_factory.conf'))
gateway_env.Install('$MUZZLEY_DISTDIR/bin', gateway_env.SConscript('src/SConscript', exports = ['gateway_env']))
gateway_env.Install('$MUZZLEY_DISTDIR/bin', File('Manifest.xml'))

