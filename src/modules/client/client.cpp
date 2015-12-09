#define MODULE_ID 2777954855
#include <Module.h>
#include <modules/client/Client.h>
#include <boost/thread.hpp>

using namespace modules::client;

ClientModule::ClientModule()
{
  MLOG("Client module constructed...");
  pInter = new ClientInter(this);
  manager = NULL;
  // Lock until it's ready
  managerInterMtx.lock();
}

ClientModule::~ClientModule()
{
  delete pInter;
}

void ClientModule::shutdown()
{
  MLOG("Shutting down persist module..");
}

void ClientModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;

  // Depend on the manager
  mInter->requireDependency(3133916783);
}

void ClientModule::injectDependency(u32 id, void* dep)
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

void ClientModule::releaseDependency(u32 id)
{
  MLOG("Dep released "<<id);
}

void* ClientModule::getPublicInterface()
{
  return pInter;
}

void ClientModule::handleEvent(u32 event, void* data)
{
}

#define REGISTER_METHOD(mod) methods.push_back(std::shared_ptr<PersistMethod>(new mod));

// Main function
void ClientModule::module_main()
{
  // We need to wait for the manager to be injected
  managerInterMtx.lock();
  managerInterMtx.unlock();
}

ClientInter::ClientInter(ClientModule * mod)
{
  this->mod = mod;
}

ClientInter::~ClientInter()
{
}

void ClientInter::handleCommand(void* command)
{
}

CHARLIE_CONSTRUCT(ClientModule);
