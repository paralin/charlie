#define MODULE_ID 1909739493
#include <Module.h>
#include <modules/directconnect/DirectConnect.h>
#include <boost/thread.hpp>

using namespace modules::directconnect;

DirectConnectModule::DirectConnectModule()
{
  MLOG("Direct connect socket net module constructed...");
  pInter = new DirectConnectInter(this);
}

DirectConnectModule::~DirectConnectModule()
{
  delete pInter;
}

void DirectConnectModule::shutdown()
{
  MLOG("Shutting down directconnect module..");
}

void DirectConnectModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;
}

void DirectConnectModule::injectDependency(u32 id, void* dep)
{
  if(!dep || !id) return;
  MLOG("Dep injected "<<id);
}

void DirectConnectModule::releaseDependency(u32 id)
{
  MLOG("Dep released "<<id);
}

void* DirectConnectModule::getPublicInterface()
{
  return pInter;
}

// Main function
void DirectConnectModule::module_main()
{
  MLOG("Initializing directconnect module...");
}

void DirectConnectModule::handleEvent(u32 eve, void* data)
{
}

DirectConnectInter::DirectConnectInter(DirectConnectModule * mod)
{
  this->mod = mod;
}

bool DirectConnectInter::ready()
{
  return false;
}


DirectConnectInter::~DirectConnectInter()
{
}

void DirectConnectInter::handleCommand(void* command)
{
}

CHARLIE_CONSTRUCT(DirectConnectModule);
