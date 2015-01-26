#include <charlie/ModuleManager.h>
#include <charlie/System.h>
#include <charlie/CryptoBuf.h>

ModuleManager::ModuleManager(System* system)
{
  sys = system;
}

ModuleManager::~ModuleManager()
{
  sys = NULL;
}

void ModuleManager::setSystemInfo(SystemInfo* info)
{
  sysInfo = info;
}

bool ModuleManager::parseModuleTable(charlie::CSignedBuffer* inbuf, charlie::CModuleTable* target)
{
  CLOG("Verifying signature and parsing module table...");
  if(verifySignedBuf(inbuf, sys->crypto, true) != SUCCESS)
  {
    CERR("Unable to verify module table signature!");
    return false;
  }
  if(!target->ParseFromArray(inbuf->data().c_str(), inbuf->data().length()))
  {
    CERR("Unable to parse module table.");
    return false;
  }
  CLOG("Module table verified and loaded.");
  return true;
}
