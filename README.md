![GuardWatch-Agent](res/guardwatch.png#center)

GuardWatch-Agent
================

## Build third-party libraries

darwin means my build platform name on macosx.

### NET SNMP

```bash

cd build/darwin

mkdir -p 3rd/net-snmp-5.9.4 && cd 3rd/net-snmp-5.9.4

../../../../3rd/net-snmp-5.9.4/configure

make
```

### SQLite

```bash

cd build/darwin

mkdir -p 3rd/sqlite-3.43.2 && cd 3rd/sqlite-3.43.2

../../../../3rd/sqlite-3.43.2/configure

make
```

### RRDTool

```bash

cd build/darwin

mkdir -p 3rd/rrdtool-1.8.0 && cd 3rd/rrdtool-1.8.0

../../../../3rd/rrdtool-1.8.0/configure

make
```

### SNMP TRAP EXAMPLE

```bash
snmptrap -v 2c -c public host:162 .1.3.6.1.6.3.1.1.5.3 .1.3.6.1.6.3.1.1.5.3 ifIndex i 2 ifAdminStatus i 1 ifOperStatus i 1
```

### Compiling Net-SNMP on MinGW

```bash

CC=x86_64-w64-mingw32-gcc ./configure --prefix="c:/netsnmp" --with-mibdirs="c:/netsnmp/mibs" --with-mib-modules="agentx disman/event-mib winExtDLL examples/example" --disable-embedded-perl --without-perl-modules --without-openssl --host=x86_64-w64-mingw32

```

### Compiling libvncserver on MinGW

```bash

cmake -G "Unix Makefiles" -DCMAKE_TOOLCHAIN_FILE=~/export/bin/toolchain-mingw32-x86_64.cmake -DWITH_OPENSSL=OFF -DWITH_GCRYPT=OFF ../..

```

###

```c
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
   //define something for Windows (32-bit and 64-bit, this part is common)
  #ifdef _WIN64
  //define something for Windows (64-bit only)
  #else
  //define something for Windows (32-bit only)
  #endif
#elif __APPLE__
    #include <TargetConditionals.h>
    #if TARGET_IPHONE_SIMULATOR
         // iOS, tvOS, or watchOS Simulator
    #elif TARGET_OS_MACCATALYST
         // Mac's Catalyst (ports iOS API into Mac, like UIKit).
    #elif TARGET_OS_IPHONE
        // iOS, tvOS, or watchOS device
    #elif TARGET_OS_MAC
        // Other kinds of Apple platforms
    #else
    #   error "Unknown Apple platform"
    #endif
#elif __linux__
    // linux
#elif __unix__ // all unices not caught above
    // Unix
#elif defined(_POSIX_VERSION)
    // POSIX
#else
#   error "Unknown compiler"
#endif
```

### rrdtool

```bash

export RRDTOOL=rrdtool-1.8.0
mkdir -p 3rd/rrdtool-1.8.0 && cd 3rd/$RRDTOOL && ../../../../3rd/$RRDTOOL/configure && make
```

## rrdtool graph

图片生成脚本

```
rrdtool graph x.png -w 785 -h 300 --start now-60 --end now --upper-limit 100 --lower-limit 0 --x-grid SECOND:5:SECOND:5:SECOND:5:0:%M\:%S DEF:cpu=./rrd/guardwatch.rrd:cpu:AVERAGE LINE1:cpu#009900:"CPU (IDLE)" GPRINT:cpu:AVERAGE:"CPU (IDLE)\: %.1lf%%"
```

效果图

![CPU示例](www/img/cpu.png)