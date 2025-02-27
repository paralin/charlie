#pragma once
#define PRINT_KEYS

#include <Common.h>
#include <Logging.h>

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string>

#include <charlie/Crypto.h>
#include <charlie/base64.h>
#include <charlie/xor.h>
#include <charlie/ModuleManager.h>
#include <charlie/machine_id.h>
#include <charlie/SystemInfo.h>
#include <charlie/ModuleTable_Data.h>
#include <charlie/ManagerModule_Data.h>

#include <protogen/charlie.pb.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/asio/ssl/detail/openssl_init.hpp>

namespace fs = boost::filesystem;

class System {
  public:
    System();
    ~System();

    int main(int argc, const char* argv[]);

    SystemInfo sysInfo;

    bool startSubprocess = false;
    bool ignoreInvalidManager = false;
    std::string subprocessPath;
    std::vector<std::string> subprocessArgs;

  private:
    void     loadRootPath(const char* arvg0);
    void     loadSysInfo();
    int      loadConfigFile();
    bool     validateConfig();
    void     serializeConfig();
    void     deserializeConfig();
    void     saveConfig();
    void     generateIdentity();
    int      loadIdentityToCrypto();
    int      loadServerPubKey();
    void     loadDefaultModuleTable();
    void     dropDefaultManager();

    ModuleManager *mManager;
    //boost::asio::ssl::detail::openssl_init<true> opensslInit;
    bool identityLoaded;

  public:
    Crypto* crypto;
    charlie::CSaveContainer config;
    boost::mutex cmtx;
    boost::mutex fsmtx;
    unsigned char*    configData;
    int      configDataSize = 0;

    void     validateAndSaveConfig();
    int      relocateEverything(const char* targetRoot, const char* targetExecutableName);
};
