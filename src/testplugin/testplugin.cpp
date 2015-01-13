#include <Module.h>

namespace modules
{
  namespace testplugin
  {
    class TestPlugin : public modules::Module
    {
      public:
        std::string toString()
        {
          return std::string("Coming from plugin");
        }
    };
  }
}
extern "C"
{
  G_MODULE_EXPORT modules::Module* construct()
  {
    return new modules::testplugin::TestPlugin();
  }
}
