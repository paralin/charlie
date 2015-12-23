#include <modules/{MODULE_NAME}/{MODULE_NAME_UC}.h>

using namespace modules::{MODULE_NAME};

{MODULE_NAME_UC}Module::{MODULE_NAME_UC}Module() :
  mInter(NULL),
  pInter(new {MODULE_NAME_UC}Inter(this))
{
  MLOG("{MODULE_NAME_UC} module constructed...");
}

{MODULE_NAME_UC}Module::~{MODULE_NAME_UC}Module()
{
  delete pInter;
}

void {MODULE_NAME_UC}Module::shutdown()
{
  MLOG("Shutting down {MODULE_NAME} module..");
  running = false;
}

void {MODULE_NAME_UC}Module::setModuleInterface(ModuleInterface* inter)
{
  mInter = inter;
}

void* {MODULE_NAME_UC}Module::getPublicInterface()
{
  return pInter;
}

int {MODULE_NAME_UC}Module::parseModuleInfo()
{
  std::string info = mInter->getModuleInfo();
  return sInfo.ParseFromString(info) == 0;
}

void {MODULE_NAME_UC}Module::module_main()
{
  parseModuleInfo();
}

void {MODULE_NAME_UC}Module::handleMessage(std::string& data)
{
  MERR("Don't know how to handle emsg " << head.emsg() << ".");
}

void {MODULE_NAME_UC}Module::handleEvent(u32 eve, void* data)
{
}

{MODULE_NAME_UC}Inter::{MODULE_NAME_UC}Inter({MODULE_NAME_UC}Module * mod)
{
  this->mod = mod;
}

{MODULE_NAME_UC}Inter::~{MODULE_NAME_UC}Inter()
{
}

void {MODULE_NAME_UC}Inter::handleCommand(const charlie::CMessageTarget& targ, std::string& command)
{
  mod->handleMessage(targ, command);
}

CHARLIE_CONSTRUCT({MODULE_NAME_UC}Module);
