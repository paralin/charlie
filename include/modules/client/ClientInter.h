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

      void handleCommand(u32 emsg, std::string& buf);

    private:
      ClientModule* mod;
    };
  }
}
