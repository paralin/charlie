#include <ModuleInterface.h>
#include <charlie/ModuleManager.h>

using namespace modules;

ModuleInterface::ModuleInterface(ModuleManager * manager, ModuleInstance * inst)
{
  this->mManager = manager;
  this->inst = inst;
}

ModuleInterface::~ModuleInterface()
{
  this->mManager = NULL;
}

//todo threadsafe
int ModuleInterface::requireDependency(u32 id)
{
  inst->modReqs.insert(id);
  mManager->evaluateRequirements();
}

void ModuleInterface::releaseDependency(u32 id)
{
  inst->modReqs.erase(id);
}
