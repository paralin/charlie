#pragma once

#include <Common.h>
#include <Logging.h>
#include <charlie/Crypto.h>
#include <charlie/base64.h>
#include <charlie/xor.h>
#include <protogen/charlie.pb.h>
#include <mongoose.h>
#include <string>

class System;
class WebHost {
  public:
    WebHost(System * sys);
    ~WebHost();

    void mainThread();

    struct mg_server* server;
    std::string info;

    System* sys;

    charlie::CModuleTable* table;
};
