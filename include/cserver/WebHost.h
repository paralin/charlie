#pragma once

#include <Common.h>
#include <Logging.h>
#include <charlie/Crypto.h>
#include <charlie/base64.h>
#include <charlie/xor.h>
#include <protogen/charlie.pb.h>
#include <mongoose.h>

class System;
class WebHost {
  public:
    WebHost(System * sys);
    ~WebHost();

    void mainThread();

    struct mg_server* server;

  private:
    std::string info;
    System* sys;
};
