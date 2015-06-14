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

#define USE_MHASH_FILENAME

ModuleManager::ModuleManager(System* system)
{
  sys = system;
  configDirty = false;
  modulesDirty = false;
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
  if(!target->ParseFromString(inbuf->data()))
  {
    CERR("Unable to parse module table.");
    return false;
  }
  CLOG("Module table verified and parsed, timestamp: "<<target->timestamp());
  return true;
}

bool ModuleManager::loadIncomingModuleTable(charlie::CSignedBuffer* buf)
{
  charlie::CModuleTable ntab;
  CLOG("Verifying incoming module table...");
  if(!parseModuleTable(buf, &ntab))
  {
    CLOG("Verification failed...");
    return false;
  }
  if(!ntab.has_timestamp())
  {
    CERR("Timestamp required on incoming table...");
    return false;
  }
  if(ntab.timestamp() <= sys->modTable.timestamp())
  {
    CERR("Incoming table is older / same as current.");
    return false;
  }
  //Okay, we have a good new module table
  sys->modTable.Clear();
  sys->modTable.CheckTypeAndMergeFrom(ntab);
  CLOG("Merged new verified module table.");
  sys->config.set_allocated_emodtable(buf);
  deferSaveConfig();
  //todo: maybe we need to update all the modules?
  //todo: notify the modules of the change?
  return true;
}

char* ModuleManager::getModuleFilename(charlie::CModule* mod)
{
  //The filename is based on the hash of the file xor by sysid
  const unsigned char* digest = (const unsigned char*)mod->hash().c_str();
  /*
  std::ostringstream ss;
  ss << mod->id();
  std::string s = ss.str();
  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, s.c_str(), s.length());
  SHA256_Final(digest, &ctx);
  */
  char mdString[SHA256_DIGEST_LENGTH*2+1];
  size_t keyi=0;
  size_t keylen = strlen(sysInfo->system_id);
  for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
  {
    sprintf(&mdString[i*2], "%02x", (unsigned int)(digest[i])^(unsigned int)sysInfo->system_id[keyi]);
    keyi++;
    if(keyi >= keylen) keyi = 0;
  }
  //Calculate filename length
  //First grab the 0-9 value of the first
  int first = (((int)mdString[0])%10);
  if(first < 2) first = 2;
  //now scale from 2-9 to 5-12
  int fnlen = (int)(((float)(first/9.0f))*7.0f + 5.0f);
  std::string fnstr (mdString);
  std::string rot_pat (sysInfo->root_path);
  CLOG(rot_pat);
  return g_module_build_path((const gchar *) (rot_pat.substr(0, rot_pat.length()-1).c_str()), (const char*)fnstr.substr(0, fnlen).c_str());
}

bool ModuleManager::moduleLoadable(u32 id, bool cleanFail)
{
  charlie::CModule* mod = findModule(id);
  if(mod == NULL)
    return false;
  moduleLoadable(mod, cleanFail);
}

bool ModuleManager::moduleLoadable(charlie::CModule* mod, bool cleanFail)
{
  //First get the filename of the module
  char* path = getModuleFilename(mod);

  //Check the path exists
  if(!boost::filesystem::exists(path)) {
    CLOG("File "<<path<<" does not exist.");
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
    if(cleanFail)
    {
      CLOG("Deleting mismatch library file...");
      boost::filesystem::remove(path);
    }
    free(path);
    free(digest);
    return false;
  }

  CLOG("Verified module "<<mod->id()<<".");

  free(path);
  free(digest);
  return true;
}

charlie::CModule* ModuleManager::findModule(u32 id, int* idx)
{
  int emcount = sys->modTable.modules_size();
  charlie::CModule *emod = NULL;
  int i;
  for(i=0;i<emcount;i++)
  {
    emod = sys->modTable.mutable_modules(i);
    if(emod->id() == id) break;
    emod = NULL;
  }
  if(idx != NULL) *idx = i;
  return emod;
}

bool ModuleManager::moduleRunning(u32 id)
{
  return minstances.count(id)>0;
}

int ModuleManager::launchModuleWithChecks(u32 id)
{
  charlie::CModule * mod = findModule(id);
  if(mod == NULL) return -1;
  return launchModuleWithChecks(mod);
}

int ModuleManager::launchModuleWithChecks(charlie::CModule* mod)
{
  //check if module loadable with a hash check
  if(!moduleLoadable(mod, true)) return 1;
  return launchModule(mod);
}

int ModuleManager::launchModule(u32 id)
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
  std::shared_ptr<ModuleInstance> inst (new ModuleInstance(mod, modPat, this));
  if(!inst->load())
  {
    CERR("Unable to load module "<<mod->id()<<"...");
    return 1;
  }
  //Iterate over existing instances
  for(auto &any : minstances)
  {
    std::shared_ptr<ModuleInstance> tinst = any.second;
    if(tinst->modReqs.count(mod->id())>0)
      tinst->notifyModuleLoaded(mod->id(), inst->publicInterface);
  }
  minstances.insert(std::pair<int, std::shared_ptr<ModuleInstance>>(mod->id(), inst));
  return 0;
}

void ModuleManager::evaluateRequirements()
{
  bool solved = false;
  std::map<int, std::shared_ptr<ModuleInstance>> solmods(minstances);
  std::set<u32> solution;
  while(!solved)
  {
    //Solution now contains all requirements
    //in solution and not in map -> add
    //in map and not in solution -> remove
    solution.clear();
    solved = true;
    for (auto &any : solmods ) {
      std::shared_ptr<ModuleInstance> inst = any.second;
      solution.insert(inst->modReqs.begin(), inst->modReqs.end());
    }
    solution.insert(tlReqs.begin(), tlReqs.end());
    std::set<u32> toRemove;
    for (auto &any : solmods )
      if(solution.count(any.first)==0)
        toRemove.insert(any.first);
    for (auto id : toRemove)
      solmods.erase(id);
    solved = toRemove.empty();
  }

  //Now merge solutions and solmods
  for (auto &any : solmods )
    solution.insert(any.first);

  CLOG("MLOAD count: "<<solution.size());

  //Now solution is the ids to keep
  for(auto &any : minstances)
    if(solution.count(any.first)==0)
      minstances.erase(any.first);

  //Load the modules we do need
  for(auto id : solution){
    if(!moduleRunning(id))
      if(launchModuleWithChecks(id) != 0)
      {CLOG("Module "<<id<<" can't be launched yet.");}
  }
}

void ModuleManager::deferRecheckModules()
{
  mtx.lock();
  CLOG("Deferring modules update...");
  modulesDirty = true;
  mtx.unlock();
}

void ModuleManager::deferSaveConfig()
{
  mtx.lock();
  CLOG("Deferring config save...");
  configDirty = true;
  mtx.unlock();
}

void ModuleManager::deferReloadModule(u32 id)
{
  mtx.lock();
  CLOG("Deferring reload of "<<id<<"...");
  toReload.insert(id);
  mtx.unlock();
}

void ModuleManager::updateEverything()
{
  mtx.lock();
  if(toReload.size()>0)
  {
    mtx.unlock();
    for(auto id : toReload)
    {
      if(moduleRunning(id))
      {
        CLOG("Reloading "<<id<<" as requested...");
        minstances.erase(id);
      }
    }
    mtx.lock();
    toReload.clear();
  }
  mtx.unlock();
  if(modulesDirty)
    evaluateRequirements();
  mtx.lock();
  if(notifyRelease.size()>0){
    for (auto &any : minstances ) {
      std::shared_ptr<ModuleInstance> inst = any.second;
      for(auto id : notifyRelease)
        inst->notifyModuleReleased(id);
    }
    notifyRelease.clear();
  }

  if(configDirty)
    sys->validateAndSaveConfig();

  modulesDirty = false;
  configDirty  = false;
  mtx.unlock();
}

void ModuleManager::onModuleReleased(u32 id)
{
  mtx.lock();
  notifyRelease.insert(id);
  mtx.unlock();
}

charlie::CModuleStorage* ModuleManager::storageForModule(u32 id)
{
  sys->cmtx.lock();
  charlie::CModuleStorage* ptr = NULL;
  bool found = false;
  int i=0;
  for(auto conf : sys->config.mod_storage())
  {
    if(conf.id() == id)
    {
      found = true;
      break;
    }
    i++;
  }
  if(!found)
  {
    ptr = sys->config.add_mod_storage();
    ptr->set_id(id);
  }
  else
  {
    ptr = sys->config.mutable_mod_storage(i);
  }
  sys->cmtx.unlock();
  return ptr;
}

ModuleInstance* ModuleManager::getModuleInstance(u32 id)
{
  if(minstances.count(id) == 0) return NULL;
  return minstances[id].get();
}
