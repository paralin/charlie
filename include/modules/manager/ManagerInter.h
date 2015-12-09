#pragma once
#include <ModuleAPI.h>
#include <string>

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
      std::string fetchUrl(const std::string& url);
      std::string fetchStaticUrl(const std::string& url);
      std::string fetchOcUrl(const std::string& url);

    private:
      ManagerModule* mod;
    };
  }
}
