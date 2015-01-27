#include <string>
#include <charlie/ModuleManager.h>
#include <charlie/System.h>
#include <charlie/CryptoBuf.h>
#include <openssl/sha.h>
#include <glib.h>
#include <gmodule.h>
#include <boost/filesystem.hpp>
#include <charlie/hash.h>
#include <sstream>

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

char* ModuleManager::getModuleFilename(charlie::CModule* mod)
{
  //The filename is base 64 encoded ID number
  std::ostringstream ss;
  ss << mod->id();
  std::string s = ss.str();
  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, s.c_str(), s.length());
  SHA256_Final(digest, &ctx);
  char mdString[SHA256_DIGEST_LENGTH*2+1];
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
    sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
  //Calculate filename length
  //First grab the 0-9 value of the first
  int first = (((int)mdString[0])%10);
  if(first < 2) first = 2;
  //now scale from 2-9 to 5-12
  int fnlen = (int)(((float)(first/9.0f))*7.0f + 5.0f);
  std::string fnstr (mdString);
  return g_module_build_path((const gchar *) sysInfo->root_path, (const char*)fnstr.substr(0, fnlen).c_str());
}

bool ModuleManager::moduleLoadable(charlie::CModule* mod)
{
  //First get the filename of the module
  char* path = getModuleFilename(mod);
  //Check the path exists
  if(!boost::filesystem::exists(path)) {
    free(path);
    return false;
  }
  //Hash the path
  unsigned char* digest;
  if(sha256File(path, &digest)!=0){
    free(path);
    return false;
  }

  //Check if it matches the module def
  const char* hash = mod->hash().c_str();
  if(memcmp(hash, digest, SHA256_DIGEST_LENGTH)!=0)
  {
    CERR("Module digest for ["<<mod->id()<<"] doesn't match.");
    free(path);
    free(digest);
    return false;
  }

  CLOG("Verified module "<<mod->id()<<".");

  free(path);
  free(digest);
  return true;
}

charlie::CModule* ModuleManager::findModule(int id, int* idx)
{
  int emcount = sys->modTable.modules_size();
  charlie::CModule *emod = NULL;
  int i;
  for(i=0;i<emcount;i++)
  {
    emod = sys->modTable.mutable_modules(i);
    if(emod->initial()) break;
    emod = NULL;
  }
  if(idx != NULL) *idx = i;
  return emod;
}

bool ModuleManager::moduleRunning(int id)
{
  return minstances.count(id)>0;
}

int ModuleManager::launchModule(int id)
{
  charlie::CModule * mod = findModule(id);
  if(mod == NULL) return -1;
  return launchModule(mod);
}

//Assumes the module exists on the disk and is verified
int ModuleManager::launchModule(charlie::CModule* mod)
{
  if(moduleRunning(mod->id())) return 0;
  char* path = getModuleFilename(mod);
  std::string modPat(path);
  free(path);
  std::shared_ptr<ModuleInstance> inst (new ModuleInstance(mod, modPat));
  if(!inst->load())
  {
    CERR("Unable to load module "<<mod->id()<<"...");
    return 1;
  }
  CLOG("Loaded manager module.");
  return 0;
}
