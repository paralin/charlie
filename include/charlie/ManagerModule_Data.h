#pragma once

extern "C" const char manager_data[];
extern "C" const size_t manager_data_len;
extern "C" const char* manager_data_key;


void decryptManagerData(char** buf);
