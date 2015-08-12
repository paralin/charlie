#pragma once
#include <ModuleAPI.h>

//This is the public interface
namespace modules
{
  namespace manager
  {
    class ManagerModule;
    class VISIBLE ManagerInter
    {
    public:
      ManagerInter(ManagerModule * mod);
      ~ManagerInter();

      bool prepareToRelocate();

    private:
      ManagerModule* mod;
    };
  }
}
