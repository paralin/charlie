#include <networking/EMsgSizes.h>

// List of maximum sizes
// todo: update these wih actual numbers
std::map<charlie::EMsg, int> charlie::networking::StaticDefinitions::EMsgSizes = {
  // 3kb
  {charlie::EMsgIdentify, 3000},
  {charlie::EMsgAccept, 3000},
};
