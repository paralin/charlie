#pragma once
#include <ModuleAPI.h>
#include <string>
#include <protogen/manager.pb.h>

//This is the public interface
namespace modules
{
  namespace manager
  {
    class ManagerModule;
    class VISIBLE ManagerInter : public ModuleAPI
    {
    public:
      ManagerInter(ManagerModule * mod);
      ~ManagerInter();

      bool prepareToRelocate();
      CManagerInfo* getInfo();
      void fetchUrl(std::string& url, std::ostream& outp);
      void handleCommand(const charlie::CMessageTarget& targ, std::string& data);

      void setOrProxy(std::string proxy, std::string auth);

    private:
      ManagerModule* mod;
    };
  }
}
