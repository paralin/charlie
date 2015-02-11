//xxx: Later maybe this can be automated?
#define MODULE_ID 3133916783
#define CHARLIE_MODULE
#include <modules/manager/Manager.h>
#include <boost/thread.hpp>

using namespace modules::manager;

ManagerModule::ManagerModule()
{
  MLOG("Manager module constructed...");
}

void ManagerModule::shutdown()
{
  MLOG("Shutting down manager module..");
}

void ManagerModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;
}

void ManagerModule::injectDependency(u32 id, void* dep)
{
  MLOG("Dep injected "<<id);
}

void ManagerModule::releaseDependency(u32 id)
{
  MLOG("Dep released "<<id);
}

bool running = true;
void ManagerModule::module_main()
{
  mInter->requireDependency(4);
  mInter->commitDepsChanges();
  while(running)
  {
    try{
      //An interruption point
      boost::this_thread::sleep( boost::posix_time::milliseconds(2000));
    }
    catch(...)
    {
      MLOG("Interrupted, exiting...");
      break;
    }
  }
}

CHARLIE_CONSTRUCT(modules::manager::ManagerModule);
