#pragma once
#include <Module.h>
#include <ModuleAPI.h>

namespace modules
{
  namespace {MODULE_NAME}
  {
    class {MODULE_NAME_UC}Module;
    class VISIBLE {MODULE_NAME_UC}Inter : public ModuleAPI
    {
    public:
      {MODULE_NAME_UC}Inter({MODULE_NAME_UC}Module * mod);
      ~{MODULE_NAME_UC}Inter();

      void handleCommand(const charlie::CMessageTarget& target, std::string& buf);
    private:
      {MODULE_NAME_UC}Module* mod;
    };
  }
}
