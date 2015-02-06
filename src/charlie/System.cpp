#include <charlie/System.h>
#include <charlie/machine_id.h>
#include <charlie/ServerKey_Data.h>
#include <charlie/ModuleTable_Data.h>
#include <charlie/ManagerModule_Data.h>
#include <openssl/sha.h>
#include <google/protobuf/repeated_field.h>
#include <boost/thread/thread.hpp>

#ifndef NDEBUG
#include <csignal>
#endif

#define FREE_OLD_CONFIG if(configDataSize>0){free(configData);configDataSize=0;}

using namespace std;

System::System(void)
{
  crypto = new Crypto();
  mManager = new ModuleManager(this);
  configDataSize = 0;
}

System::~System(void)
{
  free((char*)sysInfo.system_id);
  free((char*)sysInfo.b64_system_id);
  free((char*)sysInfo.config_filename);
  free((char*)sysInfo.exe_path);
  free((char*)sysInfo.root_path);
  delete crypto;
  delete mManager;
}

void System::loadRootPath(const char* arvg)
{
  std::string pth1 (arvg);
  std::string pth2;

  if(boost::starts_with(pth1, "./"))
    pth2 = pth1.substr(2);
  else
    pth2 = pth1;
  fs::path full_path( fs::initial_path<fs::path>() );
  full_path = fs::system_complete( fs::path( pth2 ) );

  std::string exePath = full_path.string();
  char * cexePath = (char*)malloc((exePath.length()+1)*sizeof(char));
  std::strcpy (cexePath, exePath.c_str());
  sysInfo.exe_path = (const char*)cexePath;
  CLOG("Path to exe:  " << sysInfo.exe_path);
  std::string rootPath = (full_path.remove_filename()/"/").string();
  char * crootPath = (char*)malloc(sizeof(char)*(rootPath.length()+1));
  std::strcpy (crootPath, rootPath.c_str());
  sysInfo.root_path = (const char*)crootPath;
  CLOG("Path to root: " << sysInfo.root_path);
}

int System::loadConfigFile()
{
  CLOG("Loading config file "<<sysInfo.config_filename<<".");
  if(!boost::filesystem::exists(sysInfo.config_filename))
    return -1;
  ifstream configFile;
  streampos size;
  configFile.open (sysInfo.config_filename, ios_base::in|ios_base::binary|ios_base::ate);
  if(configFile.is_open()){
    size = configFile.tellg();
    FREE_OLD_CONFIG;
    configData = (char*)malloc(sizeof(char)*size);
    configFile.seekg(0, ios_base::beg);
    configFile.read(configData, size);
    configFile.close();
    CLOG("Loaded config file, "<<size<<" length.");
    apply_xor(configData, size, sysInfo.system_id, strlen(sysInfo.system_id));
    configDataSize = (int)size;
    return SUCCESS;
  } else
    return -1;
}

void System::generateIdentity()
{
  CLOG("Generating identity...");
  charlie::CIdentity * identity = config.mutable_identity();
  if(crypto->genLocalKeyPair() != SUCCESS)
  {
    CERR("Unable to generate local key pair, continuing anyway...");
  }
  unsigned char* pkey;
  int pkeyLen = crypto->getLocalPriKey(&pkey);
  unsigned char* pubkey;
  int pubkeyLen = crypto->getLocalPubKey(&pubkey);
  if(pkeyLen == FAILURE || pubkeyLen == FAILURE)
  {
    CERR("Unable to retrieve pub/pri key, continuing anyway...");
  }
  identity->set_private_key(pkey, pkeyLen);
  identity->set_public_key(pubkey, pubkeyLen);
  CLOG("==== GENERATED KEYS ====");
  CLOG(pkey);
  CLOG(pubkey);
  CLOG("========================");
}

//Bool indicates if any changes have been made
bool System::validateConfig()
{
  cmtx.lock();
  bool dirty = false;
  CLOG("Validating config...");
  if(!config.has_identity() || !config.identity().has_private_key() || !config.identity().has_public_key())
  {
    generateIdentity();
    dirty = true;
  }
  if(!config.has_emodtable() || !mManager->parseModuleTable(config.mutable_emodtable(), &modTable))
  {
    loadDefaultModuleTable();
    dirty = true;
  }
  cmtx.unlock();
  CLOG("Config validation complete.");
  return dirty;
}

void System::serializeConfig()
{
  FREE_OLD_CONFIG;
  cmtx.lock();
  configDataSize = config.ByteSize();
  configData = (char*)malloc(sizeof(char)*configDataSize);
  if(!config.SerializeToArray(configData, configDataSize))
    CERR("Unable to serialize config to array.");
  cmtx.unlock();
}

void System::validateAndSaveConfig()
{
  CLOG("Preparing to save config...");
  validateConfig();
  serializeConfig();
  saveConfig();
}

void System::deserializeConfig()
{
  cmtx.lock();
  if(!config.ParseFromArray(configData, configDataSize))
    CERR("Unable to parse config from array.");
  cmtx.unlock();
}

void System::saveConfig()
{
  if(configDataSize == 0)
  {
    CERR("Not saving an empty configData buffer.");
    return;
  }
  char* toSave = (char*)malloc(sizeof(char)*configDataSize);
  memcpy(toSave, configData, configDataSize);
  apply_xor(toSave, configDataSize, sysInfo.system_id, strlen(sysInfo.system_id));
  CLOG("Saving config file to "<<sysInfo.config_filename);
  ofstream configFile (sysInfo.config_filename, ios_base::out|ios_base::binary);
  if(configFile.is_open()){
    configFile.write(toSave, configDataSize);
  }else{
    CERR("Unable to open config file for writing.");
  }
  configFile.close();
  free(toSave);
}

void System::loadSysInfo()
{
  sysInfo.system_id = getSystemUniqueId();
  CLOG("SystemID: "<<sysInfo.system_id);
  sysInfo.b64_system_id = base64Encode((const unsigned char*)sysInfo.system_id, strlen(sysInfo.system_id));
  size_t b64idlen = strlen(sysInfo.b64_system_id);
  CLOG("ESystemID: "<<sysInfo.b64_system_id);
  sysInfo.cpu_hash = getCpuHash();
  CLOG("CPU Hash: "<<sysInfo.cpu_hash);
  u16 filenameLen = sysInfo.cpu_hash%10;
  if(filenameLen > b64idlen) filenameLen = (u16)(b64idlen-2);
  if(filenameLen < 4) filenameLen = 4;
  std::string filename (sysInfo.b64_system_id);
  filename = filename.substr(0, filenameLen);
  CLOG("Filename: "<<filename);
  char * cstr = (char*)malloc((filename.length()+1)*sizeof(char));
  std::strcpy (cstr, filename.c_str());
  sysInfo.config_filename = (const char*)cstr;
}

int System::loadIdentityToCrypto()
{
  const std::string pkey = config.identity().private_key();
  int r1 = crypto->setLocalPriKey((unsigned char*)pkey.c_str(), pkey.length());

  const std::string pubkey = config.identity().public_key();
  int r2 = crypto->setLocalPubKey((unsigned char*)pubkey.c_str(), pubkey.length());

  if(r1 != SUCCESS || r2 != SUCCESS) return FAILURE;
  return SUCCESS;
}

int System::loadServerPubKey()
{
  CLOG("Decrypting server public key...");
  char* pub_key;
  int pub_key_len = decryptServerPubkey(&pub_key);
  if(pub_key_len == -1)
  {
    CERR("Unable to decrypt the server pub key.");
    return 1;
  }else
  {
    if(crypto->setRemotePubKey((unsigned char*)pub_key, (size_t)pub_key_len) != SUCCESS){
      CERR("Unable to set remote pubkey in crypto, continuing...");
    }
    CLOG("===== SERVER KEY =====");
    CLOG(pub_key);
    CLOG("======================");
    free(pub_key);
    return 0;
  }
}

void System::loadDefaultModuleTable()
{
  CLOG("Loading default module table...");
  if(!decryptInitModtable(config.mutable_emodtable()))
  {
    CERR("Unable to load default module table!");
    return;
  }
  if(!mManager->parseModuleTable(config.mutable_emodtable(), &modTable))
  {
    CERR("Unable to parse default module table!");
    return;
  }
}

void System::dropDefaultManager()
{
  //Load the default module table and grab the manager entry
  charlie::CModuleTable dtab;
  charlie::CSignedBuffer buf;
  if(!decryptInitModtable(&buf))
  {
    CERR("I can't drop the initial manager without the encrypted table.");
    return;
  }
  if(!mManager->parseModuleTable(&buf, &dtab))
  {
    CERR("Unable to parse default module table in dropping default manager!");
    return;
  }

  //mod is the default manager
  int mcount = dtab.modules_size();
  charlie::CModule *mod = NULL;
  for(int i=0;i<mcount;i++)
  {
    mod = dtab.mutable_modules(i);
    if(mod->initial()) break;
    mod = NULL;
  }
  if(mod == NULL)
  {
    CERR("Can't find the manager module in the default table!");
    return;
  }

  //We also need to remove any existing manager module
  int i;
  charlie::CModule *emod = mManager->findModule(MANAGER_MODULE_ID, &i);
  google::protobuf::RepeatedPtrField<charlie::CModule>* mods = modTable.mutable_modules();
  if(emod != NULL)
  {
    CERR("Removing existing initial module definition...");
    int emcount = modTable.modules_size();
    if(i != emcount-1) mods->SwapElements(i, emcount-1);
    mods->RemoveLast();
  }

  //Get the default module data decrypted
  char* dmandata;
  decryptManagerData(&dmandata);

  //Hash the default module
  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, dmandata, manager_data_len);
  SHA256_Final(digest, &ctx);

  charlie::CModule* nmod = mods->Add();
  nmod->set_id(MANAGER_MODULE_ID);
  nmod->set_initial(true);
  nmod->set_mainfcn(true);
  nmod->set_hash(digest, SHA256_DIGEST_LENGTH);

  char* path = mManager->getModuleFilename(nmod);
  CLOG("Created new module definition, dumping file to \""<<path<<"\"...");
  std::ofstream of;
  of.open(path, std::ios_base::out|std::ios_base::binary);
  of.write(dmandata, manager_data_len);
  of.close();
  free(path);
  free(dmandata);
}

bool continueLoop=true;

//Debug signal handlers
#ifndef NDEBUG
void ctrlchandler(int) {
  CLOG("Caught ctrl c, quitting...");
  continueLoop = false;
}
void killhandler(int) { 
  CLOG("Caught kill, quitting...");
  continueLoop = false;
}
#endif

int System::main(int argc, const char* argv[])
{
#ifndef NDEBUG
  signal(SIGINT, ctrlchandler);
  signal(SIGTERM, killhandler);
#endif

  loadRootPath(argv[0]);
  loadSysInfo();
  if(loadServerPubKey() != 0)
  {
    CERR("Server public key load unsuccessful, continuing anyway...");
  }
  if(loadConfigFile() != SUCCESS)
  {
    CLOG("Can't load config file, will use reasonable defaults...");
  }else
  {
    deserializeConfig();
  }
  if(validateConfig()){
    serializeConfig();
    saveConfig();
  }
  if(loadIdentityToCrypto() != SUCCESS)
  {
    CERR("Can't load the identity to crypto! Regenerating identity...");
    generateIdentity();
    serializeConfig();
    saveConfig();
  }else{
    CLOG("Loaded identity to crypto.");
  }
  mManager->setSystemInfo(&sysInfo);
  if(!mManager->moduleLoadable(mManager->findModule(MANAGER_MODULE_ID)))
  {
    CLOG("The manager module doesn't exist / failed to validate, dropping default...");
    dropDefaultManager();
  }
  CLOG("Manager module is verified, adding it as first dep!");
  mManager->tlReqs.insert(MANAGER_MODULE_ID);
  mManager->deferRecheckModules();

  CLOG("Starting main loop...");
  while(continueLoop)
  {
    mManager->updateEverything();
    boost::this_thread::sleep( boost::posix_time::milliseconds(200) );
  }
  CLOG("Exiting...");
  return 0;
}
