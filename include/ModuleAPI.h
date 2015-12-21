#pragma once
#include <Common.h>
#include <Module.h>

#include <string>
#include <iostream>

#define VISIBLE __attribute__ ((visibility ("default")))

namespace modules
{
  // An api visible to other moules
  class VISIBLE ModuleAPI
  {
  public:
    virtual ~ModuleAPI() {};

    // Handle a server command
    virtual void handleCommand(u32 emsg, std::string& buf) = 0;
  };
};
