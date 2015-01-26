#pragma once
#include <Common.h>
#include <string>
#include <iostream>
#include <glib.h>
#include <gmodule.h>
#include <vector>

namespace modules
{
  class Module
  {
  public:
    // Provide a pointer to the requested dependency
    virtual int injectDependency (u32 id, void* dep);
  };
};

#define CHARLIE_CONSTRUCT(CLASS) \
  extern "C"\
  {\
    G_MODULE_EXPORT modules::Module* construct() \
    {\
      return new CLASS();\
    }\
  }
