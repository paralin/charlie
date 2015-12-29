#pragma once
#include <Module.h>
#include <ModuleAPI.h>
#include <ModuleNet.h>

namespace modules
{
  namespace torm
  {
    class TormModule;
    class VISIBLE TormInter : public ModuleNet
    {
    public:
      TormInter(TormModule * mod);
      ~TormInter();

      void handleCommand(const charlie::CMessageTarget& target, std::string& buf);

      bool ready();
      void disconnectInvalid();
      std::shared_ptr<CharlieSession> getSession();

    private:
      TormModule* mod;
    };
  }
}
