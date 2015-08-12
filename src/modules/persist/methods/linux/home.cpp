#include <modules/persist/methods/linux/PersistHome.h>

using namespace modules::persist;

PersistHome::PersistHome()
{
}

PersistHome::~PersistHome()
{
}

void PersistHome::init(ModuleInterface* inter, PersistModule* persist)
{
  this->inter = inter;
  this->persist = persist;
}
