#pragma once
#include <Common.h>

namespace modules
{
  namespace manager
  {
    class ManagerModule;
    class CharlieClient
    {
    public:
      CharlieClient(ManagerModule* man);
      ~CharlieClient();
    private:
      ManagerModule* manager;
    };
  };
};
