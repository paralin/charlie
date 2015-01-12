#pragma once
#include <IntTypes.h>
#include <charlie/smear.h>
#include <algorithm>

void getMacHash( u16& mac1, u16& mac2 );
u16 getVolumeHash();
u16 getCpuHash();
const char* getMachineName();
u16* computeSystemUniqueId();
const char* getSystemUniqueId();
