#include <Module.h>

namespace modules
{
  namespace testplugin
  {
    class TestPlugin : public modules::Module
    {
    };
  };
};

CHARLIE_CONSTRUCT(modules::testplugin::TestPlugin);
