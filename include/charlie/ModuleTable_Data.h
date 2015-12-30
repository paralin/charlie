#pragma once
#include <protogen/charlie.pb.h>

extern "C" const char init_modtable_data[];
extern "C" const size_t init_modtable_data_len;
extern "C" const char* init_modtable_key;

bool decryptInitModtable(charlie::CModuleTable*);
