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

    // Handle a server command packet
    // This method is expected to release the buffer later
    virtual void handleCommand(void* buf) = 0;
  };
};
