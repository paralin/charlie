#pragma once
#include <Common.h>
#include <Module.h>

#include <string>
#include <iostream>

#include <protogen/charlie_net.pb.h>

#define VISIBLE __attribute__ ((visibility ("default")))

namespace modules
{
  // An api visible to other moules
  class VISIBLE ModuleAPI
  {
  public:
    virtual ~ModuleAPI() {};

    // Handle a server command
    virtual void handleCommand(const charlie::CMessageTarget& target, std::string& buf) = 0;
  };
};
