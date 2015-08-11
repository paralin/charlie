#pragma once
#include <Common.h>
#include <string>
#include <iostream>
#include <glib.h>
#include <gmodule.h>
#include <vector>
#include <ModuleInterface.h>

#define VISIBLE __attribute__ ((visibility ("default")))

#ifndef MODULE_ID
#define MODULE_ID 0
#endif

#ifdef CHARLIE_MODULE
#ifdef CHARLIE_MODULE_NAME
#define MLOG(msg) CLOG("["<<CHARLIE_MODULE_NAME<<"] "<<msg);
#define MERR(msg) CERR("["<<CHARLIE_MODULE_NAME<<"]! "<<msg);
#else
#define MLOG(msg) CLOG("["<<MODULE_ID<<"m] "<<msg);
#define MERR(msg) CERR("["<<MODULE_ID<<"m]! "<<msg);
#endif
#endif

namespace modules
{
  class VISIBLE Module
  {
  public:
    virtual ~Module() {};
    // Provide a pointer to the requested dependency
    virtual void injectDependency (u32 id, void* dep) = 0;
    // When a dep is released
    virtual void releaseDependency(u32 id) = 0;
    // Provide a handle to the module interface
    virtual void setModuleInterface(ModuleInterface* inter) = 0;
    // Release everything and prepare to be deleted
    virtual void shutdown() = 0;

    // If module->mainfcn() then this will be run
    virtual void module_main() = 0;

    virtual void* getPublicInterface() = 0;
  };
};

#define CHARLIE_CONSTRUCT(CLASS) \
  extern "C"\
  {\
    G_MODULE_EXPORT VISIBLE modules::Module* cmconstr() \
    {\
      return new CLASS();\
    }\
  }
