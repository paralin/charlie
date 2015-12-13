#pragma once
#include <Module.h>
#include <ModuleAPI.h>
#include <ModuleNet.h>

//This is the public interface
namespace modules
{
  namespace directconnect
  {
    class DirectConnectModule;
    class VISIBLE DirectConnectInter : public ModuleAPI, public ModuleNet
    {
    public:
      DirectConnectInter(DirectConnectModule * mod);
      ~DirectConnectInter();

      void handleCommand(void* buf);
      bool ready();
    private:
      DirectConnectModule* mod;
    };
  }
}
