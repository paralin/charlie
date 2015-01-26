#pragma once
#define PRINT_KEYS

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <charlie/Crypto.h>
#include <charlie/base64.h>
#include <charlie/xor.h>
#include <charlie/ModuleManager.h>
#include <protogen/charlie.pb.h>
#include <Logging.h>
#include <Common.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <charlie/machine_id.h>
#include <charlie/SystemInfo.h>
#include <charlie/ModuleTable_Data.h>
#include <charlie/ManagerModule_Data.h>

namespace fs = boost::filesystem;

class System {
  public:
    System();
    ~System();

    int main(int argc, const char* argv[]);

    SystemInfo sysInfo;

  private:
    void     loadRootPath(const char* arvg0);
    void     loadSysInfo();
    int      loadConfigFile();
    void     validateConfig();
    void     loadDefaultConfig();
    void     serializeConfig();
    void     deserializeConfig();
    void     saveConfig();
    void     generateIdentity();
    int      loadIdentityToCrypto();
    int      loadServerPubKey();
    void     loadDefaultModuleTable();

    ModuleManager *mManager;

  public:
    charlie::CSaveContainer config;
    char*    configData;
    int      configDataSize;
    charlie::CModuleTable modTable;

    Crypto* crypto;
};
