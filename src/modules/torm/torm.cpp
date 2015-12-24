#define MODULE_ID 6032034
#include <modules/torm/Torm.h>
#include <modules/torm/TorC.h>

using namespace modules::torm;

TormModule::TormModule() :
  mInter(NULL),
  pInter(new TormInter(this))
{
  MLOG("Torm module constructed...");
}

TormModule::~TormModule()
{
  delete pInter;
}

void TormModule::shutdown()
{
  MLOG("Shutting down torm module..");
}

void TormModule::setModuleInterface(ModuleInterface* inter)
{
  mInter = inter;
}

void* TormModule::getPublicInterface()
{
  return pInter;
}

void TormModule::injectDependency(u32 id, void* dep)
{
}

void TormModule::releaseDependency(u32 id)
{
}

bool TormModule::parseModuleInfo()
{
  std::string info = mInter->getModuleInfo();
  return sInfo.ParseFromString(info) == 0;
}

void TormModule::module_main()
{
  parseModuleInfo();
  torc_main();
}

void TormModule::handleEvent(u32 eve, void* data)
{
}

TormInter::TormInter(TormModule * mod)
{
  this->mod = mod;
}

TormInter::~TormInter()
{
}

void TormInter::handleCommand(const charlie::CMessageTarget& targ, std::string& command)
{
  MERR("Don't know how to handle emsg " << targ.emsg() << ".");
}

CHARLIE_CONSTRUCT(TormModule);
