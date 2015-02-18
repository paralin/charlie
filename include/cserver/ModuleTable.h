#pragma once
#include <Logging.h>
#include <charlie/xor.h>
#include <charlie/Crypto.h>
#include <protogen/charlie.pb.h>
#include <protogen/charlie_server.pb.h>
#include <protogen/manager.pb.h>
#include <charlie/CryptoBuf.h>

int generateModuleTableFromJson(const char* json, char** output, Crypto* crypt, size_t* outputSize, bool doHash, std::string libprefix, std::string libsuffix, std::string rootPath, bool doSign);
