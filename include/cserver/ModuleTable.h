#pragma once
#include <Logging.h>
#include <IntTypes.h>
#include <charlie/xor.h>
#include <charlie/Crypto.h>
#include <protogen/charlie.pb.h>
#include <protogen/charlie_server.pb.h>
#include <protogen/manager.pb.h>
#include <charlie/CryptoBuf.h>

int generateModuleTableFromJson(const char* json, unsigned char** output, Crypto* crypt, size_t* outputSize, std::string rootPath, bool doSign);
charlie::CModuleTable* generateModuleTableFromJson2(const char* json, Crypto* crypt, std::string rootPath);
const char* platformToSuffix(u32 platform);
const char* platformToPrefix(u32 platform);
