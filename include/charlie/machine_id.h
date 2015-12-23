#pragma once
#include <IntTypes.h>
#include <algorithm>
#include <string>

void getMacHash( u16& mac1, u16& mac2 );
u16 getVolumeHash();
u16 getCpuHash();
std::string getMachineName();
u16* computeSystemUniqueId();
const char* getSystemUniqueId();
