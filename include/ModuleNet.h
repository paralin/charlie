#pragma once
#include <Common.h>
#include <string>

namespace modules
{
  class VISIBLE ModuleNet
  {
    public:
      virtual ~ModuleNet() {};

      // Is this network module able to communicate?
      virtual bool ready() = 0;
  }
}
