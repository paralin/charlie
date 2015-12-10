#pragma once
#include <IntTypes.h>

#define MANAGER_MODULE_ID 3133916783
#define REGISTER_DIFFICULTY 23

const char * const ONLINE_MTABLE_KEY = "xH0KEc9e5cWOx2geoXhBef2Yxy";

#ifndef G_MODULE_PREFIX
#ifdef WIN32
#define G_MODULE_PREFIX
#else
#define G_MODULE_PREFIX "lib"
#endif
#endif

#define VISIBLE __attribute__ ((visibility ("default")))
#define NAME(NAM) __asm(NAM)

/*
 * Defines CHARLIE_WINDOWS, CHARLIE_IOS, CHARLIE_OSX, or CHARLIE_LINUX
 */
#ifdef _WIN32
//define something for Windows (32-bit and 64-bit, this part is common)
#define CHARLIE_WINDOWS
#elif __APPLE__
#include "TargetConditionals.h"
#if TARGET_IPHONE_SIMULATOR
// iOS Simulator
#define CHARLIE_IOS
#elif TARGET_OS_IPHONE
// iOS device
#define CHARLIE_IOS
#else
#define CHARLIE_OSX
#endif
#elif __linux
// linux
#define CHARLIE_LINUX
#elif __unix // all unices not caught above
// Unix
#define CHARLIE_LINUX
#elif __posix
// POSIX
#define CHARLIE_LINUX
#endif

const u32 CHARLIE_PLATFORM =
#ifdef CHARLIE_LINUX
  0x04
#elif defined CHARLIE_OSX
  0x02
/*
#elif CHARLIE_IOS
  0x??
*/
#elif defined CHARLIE_WINDOWS
  0x01
#else
#error "This platform is not supported. See Common.h."
#endif
;
