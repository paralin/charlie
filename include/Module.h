#pragma once
#include <string>
#include <iostream>
#include <glib.h>
#include <gmodule.h>

namespace modules
{
  class Module
  {
  public:
    virtual std::string toString() = 0;
  };
}
