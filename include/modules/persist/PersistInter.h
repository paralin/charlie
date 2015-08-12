#pragma once
#include <Module.h>
#include <ModuleAPI.h>

//This is the public interface
namespace modules
{
  namespace persist
  {
    class PersistModule;
    class VISIBLE PersistInter : public ModuleAPI
    {
    public:
      PersistInter(PersistModule * mod);
      ~PersistInter();

      void handleCommand(void* buf);
    private:
      PersistModule* mod;
    };
  }
}
