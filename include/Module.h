#pragma once
#include <Common.h>
#include <string>
#include <iostream>
#include <glib.h>
#include <gmodule.h>
#include <vector>

#define VISIBLE_FUNCTION __attribute__ ((visibility ("default")))

namespace modules
{
  class Module
  {
  public:
    // Provide a pointer to the requested dependency
    virtual int injectDependency (u32 id, void* dep);

    // Release everything and prepare to be deleted
    virtual void shutdown();
  };
};

#define CHARLIE_CONSTRUCT(CLASS) \
  extern "C"\
  {\
    G_MODULE_EXPORT VISIBLE_FUNCTION modules::Module* cmconstr() \
    {\
      return new CLASS();\
    }\
  }
