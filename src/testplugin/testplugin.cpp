#include <Plugin.h>

namespace plugins
{
  class AwesomePlugin : public Plugin
  {
    public:
      std::string toString()
      {
        return std::string("Coming from plugin");
      }
  };
}

extern "C"
{
  G_MODULE_EXPORT plugins::Plugin* construct()
  {
    return new plugins::AwesomePlugin();
  }
}
