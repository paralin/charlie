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

namespace fs = boost::filesystem;

class System {
  public:
    System();
    ~System();

    int main(int argc, const char* argv[]);

    int loadConfigFile();

  private:
    fs::path rootPath;
    fs::path exePath;
    void loadRootPath(const char* arvg0);
};
