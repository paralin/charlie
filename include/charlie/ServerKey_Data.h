#pragma once

extern "C" const char server_pubkey_data[];
extern "C" const size_t server_pubkey_data_len;
extern "C" const char* server_pubkey_key;

int decryptServerPubkey(char** output);
