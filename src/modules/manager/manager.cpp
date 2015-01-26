#include <Module.h>

namespace modules
{
  namespace manager
  {
    class ManagerModule : public modules::Module
    {
    };
  };
};

CHARLIE_CONSTRUCT(modules::manager::ManagerModule);
