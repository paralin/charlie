#pragma once
#define PRINT_KEYS

#include <stdio.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <charlie/Crypto.h>
#include <charlie/base64.h>
#include <charlie/xor.h>
#include <proto/charlie.pb.h>
#include <Logging.h>
#include <Common.h>
#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/lexical_cast.hpp>
#include <charlie/machine_id.h>
#include <charlie/SystemInfo.h>

#ifdef DO_CHARLIE_LOG
#include <google/protobuf/text_format.h>
#endif

namespace fs = boost::filesystem;

class System {
  public:
    System();
    ~System();

    int main(int argc, const char* argv[]);

  private:
    SystemInfo sysInfo;

    void     loadRootPath(const char* arvg0);
    void     loadSysInfo();
    int      loadConfigFile();
    void     validateConfig();
    void     loadDefaultConfig();
    void     serializeConfig();
    void     deserializeConfig();
    void     saveConfig();
    int      loadIdentityToCrypto();

    char*    configData;
    int      configDataSize;
    charlie::CSaveContainer config;

    Crypto* crypto;
};
