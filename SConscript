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

Import('gateway_env')

gateway_env['MUZZLEY_DISTDIR'] = gateway_env['DISTDIR'] + '/MuzzleyConnector'

gateway_env.Append(LIBPATH = '$MUZZLEY_DISTDIR/lib');
gateway_env.Append(CPPPATH = '$MUZZLEY_DISTDIR/inc');


#RAPIDJSON
gateway_env.Append(CPPPATH = ['/home/muzzley/Libraries/rapidjson/include'])

#LIBSTROPHE
#gateway_env.Append(CPPPATH = '/usr/mips-openwrt-linux-uclibc/include')
#gateway_env.Append(LIBPATH = '/usr/mips-openwrt-linux-uclibc/lib')

#MOSQUITTO
gateway_env.Append(CPPPATH = '/usr/mips-openwrt-linux-uclibc/include')
gateway_env.Append(LIBPATH = '/usr/mips-openwrt-linux-uclibc/lib')
gateway_env.Append(LIBS=['mosquitto'])


# Set the flag True for standalone connector executable
gateway_env.Append(NO_AJ_GATEWAY = False)


gateway_env.Install('$MUZZLEY_DISTDIR/inc/transport', gateway_env.Glob('src/transport/*.h'))
gateway_env.Install('$MUZZLEY_DISTDIR/inc/common', gateway_env.Glob('src/common/*.h'))
gateway_env.Install('$MUZZLEY_DISTDIR/inc/app', gateway_env.Glob('src/app/*.h'))
gateway_env.Install('$MUZZLEY_DISTDIR/conf', gateway_env.Glob('conf/*.*'))
gateway_env.Install('$MUZZLEY_DISTDIR/bin', gateway_env.SConscript('src/SConscript', exports = ['gateway_env']))
gateway_env.Install('$MUZZLEY_DISTDIR/bin', File('Manifest.xml'))


