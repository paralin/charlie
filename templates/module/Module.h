#pragma once

#include <Common.h>
#include <Module.h>
#include <Logging.h>
#include <IntTypes.h>

#include "{MODULE_NAME_UC}Inter.h"

#include <charlie/Crypto.h>
#include <charlie/CryptoBuf.h>

#include <protogen/charlie.pb.h>
#include <protogen/client.pb.h>

namespace modules
{
  namespace {MODULE_NAME}
  {
    class {MODULE_NAME_UC}Module : public modules::Module
    {
    public:
      {MODULE_NAME_UC}Module();
      ~{MODULE_NAME_UC}Module();

      void shutdown();
      void setModuleInterface(ModuleInterface* inter);
      void injectDependency(u32 id, void* dep);
      void releaseDependency(u32 id);
      void* getPublicInterface();
      void handleEvent(u32 event, void* data);
      void module_main();

    private:
      {MODULE_NAME_UC}Inter *pInter;
      ModuleInterface* mInter;

      C{MODULE_NAME_UC}Info sInfo;

      void handleMessage(charlie::CMessageTarget& targ, std::string& data);
      void parseModuleInfo();
    };
  };
};
