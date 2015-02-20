//xxx: Later maybe this can be automated?
#define MODULE_ID 3133916783
#include <modules/manager/Manager.h>
#include <boost/thread.hpp>

using namespace modules::manager;
using namespace boost::network;

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

int ManagerModule::fetchStaticModTable()
{
  MLOG("Fetching initial module tables from all sources...");
  for(auto str : sInfo.init_url())
  {
    MLOG("Trying to fetch table from "<<str<<"...");
    try
    {
      http::client::request request(str);
      request << header("Cookie", "disclaimer_accepted=true");
      request << header("Connection", "close");
      http::client::response response =
        client.get(request);
      auto body_ = body(response);
      MLOG("Body: "<<body_);
    }catch(std::exception ex)
    {
      MERR("Unable to load URL "<<str);
    }
  }
  MLOG("Done");
  return -1;
}

int ManagerModule::parseModuleInfo()
{
  std::string info = mInter->getModuleInfo();
  return sInfo.ParseFromString(info) == 0;
}

bool running = true;
void ManagerModule::module_main()
{
  //Require persist module
  mInter->requireDependency(2526948902);
  mInter->commitDepsChanges();
  //mInter->relocateEverything("/tmp/testdir/");
  if(parseModuleInfo() != 0)
  {
    MERR("Unable to load module info...");
  }
  if(fetchStaticModTable() != 0)
  {
    MERR("Unable to fetch static module table from the internet...");
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
