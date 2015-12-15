#pragma once

#include <Common.h>
#include <Logging.h>

#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>
#include <protogen/manager.pb.h>
#include <protogen/directconnect.pb.h>

class System;
class NetHost {
  public:
    NetHost(System* sys);
    ~NetHost();

    System* sys;

    void mainThread();
};
