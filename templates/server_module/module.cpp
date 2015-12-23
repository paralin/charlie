#include <server_modules/{MODULE_NAME}/{MODULE_NAME_UC}.h>

using namespace server_modules::{MODULE_NAME};

{MODULE_NAME_UC}Module::{MODULE_NAME_UC}Module() :
  mInter(NULL)
{
  MLOG("{MODULE_NAME_UC} module constructed...");
}

void {MODULE_NAME_UC}Module::shutdown()
{
}

void {MODULE_NAME_UC}Module::setModuleInterface(ServerModuleInterface* inter)
{
  mInter = inter;
}

void {MODULE_NAME_UC}Module::inject(u32 id, void* dep)
{
}

void {MODULE_NAME_UC}Module::release(u32 id)
{
}

void {MODULE_NAME_UC}Module::handleEvent(u32 event, void* data)
{
}

void {MODULE_NAME_UC}Module::module_main()
{
}

void {MODULE_NAME_UC}Module::handleMessage(charlie::CMessageTarget* target, std::string& data)
{
}

u32 {MODULE_NAME_UC}Module::getModuleId()
{
  return {MODULE_ID};
}

{MODULE_NAME_UC}Module::~{MODULE_NAME_UC}Module()
{
}


CHARLIE_CONSTRUCT({MODULE_NAME_UC}Module);
