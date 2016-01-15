#pragma once
#include <Common.h>
#include <string>
#include <iostream>
#include <glib.h>
#include <gmodule.h>
#include <vector>

#include <SModuleInterface.h>
#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>

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

namespace server_modules
{
  class VISIBLE ServerModule
  {
  public:
    virtual ~ServerModule() {};
    // Provide a pointer to the other module
    virtual void inject(u32 id, void* dep) = 0;
    // When a module is unloaded
    virtual void release(u32 id) = 0;
    // Provide a handle to the module interface
    virtual void setModuleInterface(SModuleInterface* inter) = 0;
    // Get module id
    virtual u32  getModuleId() = 0;
    // Release everything and prepare to be deleted
    virtual void shutdown() = 0;
    // Leave this empty if you dont want it
    virtual void module_main() = 0;
    // Handle an event
    virtual void handleEvent(u32 event, void* data) = 0;
    // Handle a message from the client
    virtual void handleMessage(charlie::CMessageTarget* target, std::string& data) = 0;
  };
};

#define SEND_EMPTYMSG(CLSS, EMSG) \
  { \
    CLSS nfo; \
    std::string dt = nfo.SerializeAsString(); \
    mInter->send(getModuleId(), EMSG, 0, dt); \
  }

#define CHARLIE_CONSTRUCT(CLASS) \
  extern "C"\
  {\
    G_MODULE_EXPORT VISIBLE server_modules::ServerModule* cmconstr() \
    {\
      return new CLASS();\
    }\
  }
