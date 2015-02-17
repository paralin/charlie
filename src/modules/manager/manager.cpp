//xxx: Later maybe this can be automated?
#define MODULE_ID 3133916783
#include <modules/manager/Manager.h>
#include <boost/thread.hpp>

using namespace modules::manager;

ManagerModule::ManagerModule()
{
  MLOG("Manager module constructed...");
  pInter = new ManagerInter(this);
}

void ManagerModule::shutdown()
{
  MLOG("Shutting down manager module..");
  delete pInter;
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
  //Require persist module
  mInter->requireDependency(2526948902);
  mInter->commitDepsChanges();
  //mInter->relocateEverything("/tmp/testdir/");
  {
    std::string info = mInter->getModuleInfo();
    if(!sInfo.ParseFromString(info))
    {
      MERR("Unable to parse info!");
    }
    MLOG("Fetching initial module tables from all sources...");
    for(auto str : sInfo.init_url())
    {
      MLOG("Initial data URL: "<<str);
    }
  }
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

void* ManagerModule::getPublicInterface()
{
  return (void*)pInter;
}

ManagerInter::ManagerInter(ManagerModule* mod)
{
  this->mod = mod;
}


CHARLIE_CONSTRUCT(ManagerModule);
