//xxx: Later maybe this can be automated?
#define MODULE_ID 3133916783
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

std::string random_string( size_t length )
{
  auto randchar = []() -> char
  {
    const char charset[] =
      "0123456789"
      "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
      "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[ rand() % max_index ];
  };
  std::string str(length,0);
  std::generate_n( str.begin(), length, randchar );
  return str;
}

void ManagerModule::parseStorage()
{
  MLOG("Parsing storage...");
  if(stor->buf().data().length() == 0)
  {
    std::string rand = random_string(10);
    MLOG("Storage is empty, saving random string "<<rand<<"...");
    config.set_test(rand);
    size_t size = config.ByteSize();
    char* buf = (char*)malloc(size);
    config.SerializeToArray(buf, size);
    mInter->saveStorage(buf, size);
    free(buf);
    MLOG("Will request restart...");
    mInter->requestReload();
  }else
  {
    MLOG("Parsing config...");
    config.ParseFromArray(stor->buf().data().c_str(), stor->buf().data().length());
    MLOG("The saved string is: "<<config.test());
  }
}

bool running = true;
void ManagerModule::module_main()
{
  mInter->requireDependency(4);
  mInter->commitDepsChanges();
  stor = mInter->getStorage();
  parseStorage();
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
