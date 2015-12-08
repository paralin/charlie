#pragma once
#include <Module.h>
#include <ModuleAPI.h>

//This is the public interface
namespace modules
{
  namespace client
  {
    class ClientModule;
    class VISIBLE ClientInter : public ModuleAPI
    {
    public:
      ClientInter(ClientModule * mod);
      ~ClientInter();

      // Handle a generic command
      void handleCommand(void* buf);
    private:
      ClientModule* mod;
    };
  }
}
