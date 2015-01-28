#pragma once
#include <Common.h>
#include <string>
#include <iostream>
#include <glib.h>
#include <gmodule.h>
#include <vector>
#include <ModuleInterface.h>

#define VISIBLE_FUNCTION __attribute__ ((visibility ("default")))

#ifndef MODULE_ID
#define MODULE_ID 0
#endif

#ifdef CHARLIE_MODULE
#define MLOG(msg) CLOG("["<<MODULE_ID<<"m] "<<msg);
#define MERR(msg) CERR("["<<MODULE_ID<<"m]! "<<msg);
#endif

namespace modules
{
  class Module
  {
  public:
    // Provide a pointer to the requested dependency
    virtual void injectDependency (u32 id, void* dep);
    // When a dep is released
    virtual void releaseDependency(u32 id);
    // Provide a handle to the module interface
    virtual void setModuleInterface(ModuleInterface* inter);
    // Release everything and prepare to be deleted
    virtual void shutdown();

    // If module->mainfcn() then this will be run
    virtual void module_main();
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
