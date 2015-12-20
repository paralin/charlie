#pragma once
#include <map>
#include <protogen/charlie_net.pb.h>

namespace charlie
{
  namespace networking
  {
    class StaticDefinitions
    {
    public:
      static std::map<charlie::EMsg, int> EMsgSizes;
    };
  }
}
