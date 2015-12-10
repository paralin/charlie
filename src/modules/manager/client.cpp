#include <modules/manager/Client.h>
#include <modules/manager/Manager.h>

using namespace modules::manager;

CharlieClient::CharlieClient(ManagerModule* mod)
{
  this->manager = mod;
}

CharlieClient::~CharlieClient()
{
}
