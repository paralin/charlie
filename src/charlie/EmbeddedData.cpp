#include <Logging.h>
#include <charlie/xor.h>
#include <stdio.h>
#include <string.h>

#include <charlie/ModuleTable_Data.h>
#include <charlie/ServerKey_Data.h>
#include <charlie/ManagerModule_Data.h>

extern "C"
{
  const char* init_modtable_key = "ittybittytitty";
  const char* server_pubkey_key = "serveridentity";
  const char* manager_data_key = "GpVDIRK4KfsYC9WGbyXZ";
}

bool decryptInitModtable(charlie::CSignedBuffer* outp)
{
  char* output = (char*)malloc(sizeof(char)*init_modtable_data_len);
  memcpy(output, init_modtable_data, init_modtable_data_len);
  //Decrypt xor
  apply_xor(output, init_modtable_data_len, init_modtable_key, strlen(init_modtable_key));
  //Parse out the rsa buffer
  bool success = outp->ParseFromArray(output, init_modtable_data_len);
  free(output);
  return success;
};

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

void decryptManagerData(char** buf)
{
  *buf = (char*)malloc(sizeof(char)*manager_data_len);
  memcpy(*buf, manager_data, manager_data_len);
  apply_xor(*buf, manager_data_len, manager_data_key, strlen(manager_data_key));
}
