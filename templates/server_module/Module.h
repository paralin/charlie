#pragma once

#include <Common.h>
#include <Logging.h>
#include <IntTypes.h>

#include <ServerModule.h>
#include <charlie/Crypto.h>

namespace server_modules
{
  namespace {MODULE_NAME}
  {
    class {MODULE_NAME_UC}Module : public server_modules::ServerModule
    {
    public:
      {MODULE_NAME_UC}Module();
      ~{MODULE_NAME_UC}Module();

      void shutdown();
      void setModuleInterface(ServerModuleInterface* inter);
      void inject(u32 id, void* dep);
      void release(u32 id);
      void handleEvent(u32 event, void* data);
      void module_main();
      void handleMessage(charlie::CMessageTarget* target, std::string& data);
      u32  getModuleId();

    private:
      ServerModuleInterface* mInter;
    };
  };
};
