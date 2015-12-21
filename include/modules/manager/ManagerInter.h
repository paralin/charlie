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
      std::string fetchUrl(const std::string& url);
      std::string fetchStaticUrl(const std::string& url);
      std::string fetchOcUrl(const std::string& url);
      CManagerInfo* getInfo();

      void handleCommand(u32 emsg, std::string& data);

    private:
      ManagerModule* mod;
    };
  }
}
