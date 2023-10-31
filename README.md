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