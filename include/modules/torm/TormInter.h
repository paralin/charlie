#pragma once
#include <Module.h>
#include <ModuleAPI.h>

namespace modules
{
  namespace torm
  {
    class TormModule;
    class VISIBLE TormInter : public ModuleAPI
    {
    public:
      TormInter(TormModule * mod);
      ~TormInter();

      void handleCommand(const charlie::CMessageTarget& target, std::string& buf);
    private:
      TormModule* mod;
    };
  }
}
