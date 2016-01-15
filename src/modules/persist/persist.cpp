#define MODULE_ID 2526948902
#include <Module.h>
#include <modules/persist/Persist.h>
#include <boost/thread.hpp>

#ifndef NDEBUG
#define NO_MIGRATE
#else
#define FORCE_MIGRATE
#endif

#ifdef FORCE_MIGRATE
#undef NO_MIGRATE
#endif

// Linux persist methods
#include <modules/persist/methods/linux/PersistAutostart.h>

using namespace modules::persist;

PersistModule::PersistModule()
{
  MLOG("Persist module constructed...");
  pInter = new PersistInter(this);
  manager = NULL;
  // Lock until it's ready
  managerInterMtx.lock();
}

PersistModule::~PersistModule()
{
  delete pInter;
}

void PersistModule::shutdown()
{
  MLOG("Shutting down persist module..");
}

void PersistModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;

  // Depend on the manager
  mInter->requireDependency(3133916783);
}

void PersistModule::injectDependency(u32 id, void* dep)
{
  if(!dep) return;
  MLOG("Dep injected "<<id);
  switch(id)
  {
    case (u32)3133916783:
      managerInterMtx.unlock();
      manager = (modules::manager::ManagerInter*)dep;
      break;
  }
}

void PersistModule::releaseDependency(u32 id)
{
  MLOG("Dep released "<<id);
}

void* PersistModule::getPublicInterface()
{
  return pInter;
}

#define REGISTER_METHOD(mod) methods.push_back(std::shared_ptr<PersistMethod>(new mod));

// Main function
void PersistModule::module_main()
{
  MLOG("Waiting for manager to be injected...");

  // We need to wait for the manager to be injected
  managerInterMtx.lock();
  managerInterMtx.unlock();

  // This is ordered by priority
  // Register all of our persist methods
#ifdef CHARLIE_PersistAutostart_AVAILABLE
  REGISTER_METHOD(PersistAutostart);
#endif

  // Init all of the methods
  for(auto &i : methods)
    i->init(mInter, this);

  // Figure out what our current method is
  std::shared_ptr<PersistMethod> activeMethod;
  int ix = -1;
  for(auto &i : methods)
  {
    ix++;
    if(i->inUse())
    {
      activeMethod = i;
      break;
    }
  }

  // Is this method obviously the best?
  // If so, let's just stay the way we are.
  if(activeMethod && ix == 0)
  {
    MLOG("We are already using the best persist method.");
    return;
  }

  // Let's filter out the methods that won't work.
  methods.erase(std::remove_if(methods.begin(), methods.end(), [](std::shared_ptr<PersistMethod> i){ return !i->canUse();}), methods.end());
  if(methods.size() == 0)
  {
    MERR("There are no persist methods that work on this system.");
    return;
  }

  // Now, let's pick the first one and see if it's already in use
  if(activeMethod && methods[0] == activeMethod)
  {
    MLOG("We are already using the best persist method.");
    return;
  }

  // While we're not persisting and there are still options
  std::shared_ptr<PersistMethod> newMethod;
  while (methods.size())
  {
    newMethod = methods[0];
    if (newMethod->setup())
      break;

    // Setup failed, clear
    newMethod.reset();
    methods.erase(methods.begin());
    newMethod = NULL;
  }

  if(!newMethod && !activeMethod)
  {
    MERR("Persist failed, no supported methods for this system.");
  } else if (newMethod && activeMethod && newMethod != activeMethod) {
    if(activeMethod)
      activeMethod->cleanup();
    activeMethod.reset();
  }
}

void PersistModule::startMigrateTo(boost::filesystem::path& path, std::string& targetExecutableName)
{
#ifdef NO_MIGRATE
  MLOG("Debug mode, not actually migrating to " << path.string() << "...");
#else
#ifdef __WIN32
  MLOG("Unknown linkage error here on win32, not actually migrating.");
#else
  if (manager->prepareToRelocate()){
    mInter->relocateEverything(path.string().c_str(), (const char*) targetExecutableName.c_str());
  }
#endif
#endif
}

void PersistModule::handleEvent(u32 event, void* data)
{
}

PersistInter::PersistInter(PersistModule * mod)
{
  this->mod = mod;
}

PersistInter::~PersistInter()
{
}

void PersistInter::handleCommand(const charlie::CMessageTarget& targ, std::string& data)
{
  MERR("Received command " << targ.emsg() << " for this module but no commands are defined.");
}

CHARLIE_CONSTRUCT(PersistModule);
