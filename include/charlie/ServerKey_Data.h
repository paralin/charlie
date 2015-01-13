#pragma once
#include <stdio.h>
#include <string.h>
#include <proto/charlie.pb.h>
#include <charlie/xor.h>

extern "C" const char server_pubkey_data[];
extern "C" const size_t server_pubkey_data_len;
const char* server_pubkey_key = "serveridentity";

int decryptServerPubkey(char** output)
{
  charlie::CIdentity ident;
  char* buf = (char*)malloc(sizeof(char)*server_pubkey_data_len);
  memcpy(buf, server_pubkey_data, server_pubkey_data_len);
  //Decrypt xor
  apply_xor(buf, server_pubkey_data_len, server_pubkey_key, strlen(server_pubkey_key));
  if(!ident.ParseFromArray(buf, server_pubkey_data_len))
    return -1;
  const std::string pubkey = ident.public_key();
  *output = (char*)malloc(sizeof(char)*pubkey.length()+1);
  memcpy(*output, pubkey.c_str(), pubkey.length());
  free(buf);
  return pubkey.length()+1;
};
