#pragma once

//This is the public interface
namespace modules
{
  namespace manager
  {
    class ManagerModule;
    class ManagerInter
    {
    public:
      ManagerInter(ManagerModule * mod);

    private:
      ManagerModule* mod;
    };
  }
}
