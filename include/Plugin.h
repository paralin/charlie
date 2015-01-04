#pragma once
#include <string>
#include <iostream>
#include <glib.h>
#include <gmodule.h>

namespace plugins
{
  class Plugin
  {
  public:
    virtual std::string toString() = 0;
  };
}
