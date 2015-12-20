#include <networking/EMsgSizes.h>

// List of maximum sizes
// todo: update these wih actual numbers
std::map<charlie::EMsg, int> charlie::networking::StaticDefinitions::EMsgSizes = {
  // 3kb
  {charlie::EMsgServerIdentify, 3000},
  {charlie::EMsgClientIdentify, 3000},
  {charlie::EMsgServerAccept, 3000},
  {charlie::EMsgClientAccept, 3000}
};
