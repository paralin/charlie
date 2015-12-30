#include <charlie/System.h>
#include <charlie/machine_id.h>
#include <charlie/ServerKey_Data.h>
#include <charlie/ModuleTable_Data.h>
#include <charlie/ManagerModule_Data.h>
#include <openssl/sha.h>
#include <google/protobuf/repeated_field.h>
#include <boost/asio.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lexical_cast.hpp>
#include <unistd.h>

#ifdef DEBUG
#include <csignal>
#endif

#define FREE_OLD_CONFIG if(configDataSize>0){free(configData);configDataSize=0;}

using namespace std;
using namespace boost::asio;

volatile bool continueLoop=true;

System::System(void)
{
  identityLoaded = false;
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
  chdir(sysInfo.root_path);
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
    configData = (unsigned char*)malloc(sizeof(unsigned char)*size);
    configFile.seekg(0, ios_base::beg);
    configFile.read((char*)configData, size);
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
  if (identityLoaded)
  {
    CERR("generateIdentity() when identity already loaded.");
    return;
  }
  CLOG("Generating identity...");
  charlie::CIdentity * identity = config.mutable_identity();
  if(crypto->genLocalKeyPair() != SUCCESS)
  {
    CERR("Unable to generate local key pair, continuing anyway...");
  }
  identityLoaded = true;
  unsigned char* pkey;
  int pkeyLen = crypto->getLocalPriKey(&pkey);
  unsigned char* pubkey;
  int pubkeyLen = crypto->getLocalPubKey(&pubkey);
  if(pkeyLen == FAILURE || pubkeyLen == FAILURE)
  {
    CERR("Unable to retrieve pub/pri key, continuing anyway...");
  }
  identity->set_private_key((char*) pkey);
  identity->set_public_key((char*) pubkey);
  CLOG("==== GENERATED KEYS ====");
  CLOG(pkey);
  CLOG(pubkey);
  CLOG("========================");
  free(pkey);
  free(pubkey);
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
  if(!config.has_emodtable() || config.emodtable().signed_modules_size() == 0)
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
  configData = (unsigned char*)malloc(sizeof(unsigned char)*configDataSize);
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
  unsigned char* toSave = (unsigned char*)malloc(sizeof(unsigned char)*configDataSize);
  memcpy(toSave, configData, configDataSize);
  apply_xor(toSave, configDataSize, sysInfo.system_id, strlen(sysInfo.system_id));
  CLOG("Saving config file to "<<sysInfo.config_filename);
  ofstream configFile (sysInfo.config_filename, ios_base::out|ios_base::binary);
  if(configFile.is_open()){
    configFile.write((const char*)toSave, configDataSize);
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
  if(sysInfo.cpu_hash > 65150) sysInfo.lock_port = 55253;
  else if(sysInfo.cpu_hash < 4242) sysInfo.lock_port = 10050;
  else sysInfo.lock_port = sysInfo.cpu_hash;
}

int System::loadIdentityToCrypto()
{
  {
    const std::string pkey = config.identity().private_key();
    int r1 = crypto->setLocalPriKey((unsigned char*)pkey.c_str(), pkey.length());

    if(r1 != SUCCESS) return FAILURE;

    CLOG("==== LOADED KEYS pk ====");
    CLOG(pkey);
    CLOG("========================");
  }

  identityLoaded = true;

#ifdef DO_CHARLIE_LOG
  {
    unsigned char* pkey;
    int pkeyLen = crypto->getLocalPriKey(&pkey);
    unsigned char* pubkey;
    int pubkeyLen = crypto->getLocalPubKey(&pubkey);
    if(pkeyLen == FAILURE || pubkeyLen == FAILURE)
    {
      CERR("Unable to retrieve pub/pri key, continuing anyway...");
    }
    CLOG("==== LOADED KEYS cr ====");
    CLOG(pkey);
    CLOG(pubkey);
    CLOG("========================");
    free(pkey);
    free(pubkey);
  }
#endif

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
}

void System::dropDefaultManager()
{
  //Load the default module table and grab the manager entry
  charlie::CModuleTable dtab;
  if(!decryptInitModtable(&dtab))
  {
    CERR("I can't drop the initial manager without the encrypted table.");
    return;
  }

  //mod is the default manager
  std::shared_ptr<charlie::CModule> mod;
  {
    int mcount = dtab.signed_modules_size();
    for(int i=0;i<mcount;i++)
    {
      mod = std::make_shared<charlie::CModule>();
      if (!mod->ParseFromString(dtab.signed_modules(i).data()))
      {
        CERR("Unable to parse module from initial module table");
        mod.reset();
        continue;
      }
      if (mod->initial())
        break;
      mod.reset();
    }
    if(!mod)
    {
      CERR("Can't find the manager module in the default table!");
      return;
    }
  }

  int i;
  std::shared_ptr<charlie::CModule> emod = mManager->findModule(MANAGER_MODULE_ID, &i);

  //Get the default module data decrypted
  unsigned char* dmandata;
  decryptManagerData(&dmandata);

  //Hash the default module
  unsigned char digest[SHA256_DIGEST_LENGTH];
  SHA256_CTX ctx;
  SHA256_Init(&ctx);
  SHA256_Update(&ctx, dmandata, manager_data_decomp_len);
  SHA256_Final(digest, &ctx);

  charlie::CModuleBinary* bin = ModuleManager::selectBinary(emod);
  if (!bin)
  {
    CERR("Unable to find the hardcoded module for this platform.");
    return;
  }
  if(memcmp(digest, emod->binary(0).hash().c_str(), SHA256_DIGEST_LENGTH)!=0)
  {
    CLOG("Default manager hash doesn't match default module table manager hash...");

    //We also need to remove any existing manager module
    auto mods = config.mutable_emodtable()->mutable_signed_modules();
    if(emod != NULL)
    {
      CERR("Removing existing initial module definition...");
      int emcount = config.mutable_emodtable()->signed_modules_size();
      if(i != emcount-1) mods->SwapElements(i, emcount-1);
      mods->RemoveLast();
    }

    charlie::CSignedBuffer* nmodb = mods->Add();
    charlie::CModule nmod;
    nmod.set_id(MANAGER_MODULE_ID);
    nmod.set_initial(true);
    nmod.set_mainfcn(true);
    bin = nmod.add_binary();
    bin->set_hash(digest, SHA256_DIGEST_LENGTH);
    bin->set_platform(CHARLIE_PLATFORM);
    if(mod->has_info())
      nmod.set_info(emod->info());
    CLOG("Created new module definition...");
    nmodb->set_data(nmod.SerializeAsString());
    ignoreInvalidManager = true;
  }

  char* path = mManager->getModuleFilename(mod);
  CLOG("Dropping module to \""<<path<<"\"...");
  std::ofstream of;
  of.open(path, std::ios_base::out|std::ios_base::binary);
  of.write((const char*)dmandata, manager_data_decomp_len);
  of.close();
  free(path);
  free(dmandata);
}

int System::relocateEverything(const char* targetRoot, const char* targetExecutableName)
{
  fsmtx.lock();

  CLOG("Relocating to "<<targetRoot<<"...");
  fs::path target (targetRoot);
  fs::path croot = fs::path(sysInfo.root_path);
  fs::path targetExecutablePath;
  std::vector<std::string> filesToRemove;

  //Check if we even need to move anything
  if(fs::equivalent(target, croot))
  {
    CLOG("Target dir is equal to current root! Doing nothing.");
    fsmtx.unlock();
    return 0;
  }

  //Build the file structure
  try
  {
    fs::create_directories(target);
  }catch(...)
  {
    CERR("Unable to create target directory...");
    fsmtx.unlock();
    return 1;
  }

  //Copy the modules
  int modscount = config.mutable_emodtable()->signed_modules_size();
  for(int i=0; i<modscount; i++)
  {
    const charlie::CSignedBuffer& buf = config.emodtable().signed_modules(i);
    std::shared_ptr<charlie::CModule> mod = std::make_shared<charlie::CModule>();
    if (!mod->ParseFromString(buf.data()))
    {
      CERR("Unable to parse one of the stored modules.");
      continue;
    }
    char* fns = mManager->getModuleFilename(mod);
    if (fns == NULL)
      continue;
    fs::path fn = fs::path(fns).filename();
    free(fns);
    fs::path mpth = croot/fn;
    CLOG("Checking for "<<mpth<<"...");
    if(fs::exists(mpth))
    {
      fs::path tpth = target/fn;
      CLOG("Copying "<<mpth.string()<<" to "<<tpth.string()<<"...");
      try
      {
        fs::copy_file(mpth, tpth);
        filesToRemove.push_back(mpth.string());
      }
      catch(...)
      {
        CERR("Unable to copy "<<mod->id()<<"!");
      }
    }
  }

  //Copy the executable
  {
    fs::path epth (sysInfo.exe_path);
    fs::path exeFilename = targetExecutableName != NULL ? fs::path(targetExecutableName) : epth.filename();
    targetExecutablePath = target/exeFilename;
    try
    {
      fs::copy_file(epth, targetExecutablePath);
      filesToRemove.push_back(epth.string());
    }
    catch(...)
    {
      CERR("Unable to copy executable!");
      fsmtx.unlock();
      return 2;
    }
  }

  //Copy the config
  {
    fs::path cfn (sysInfo.config_filename);
    fs::path cpth = croot/cfn;
    fs::path tpth = target/cfn;
    try
    {
      CLOG("Copying "<<cpth<<" to "<<tpth);
      fs::copy_file(cpth, tpth);
      filesToRemove.push_back(cpth.string());
    }
    catch(...)
    {
      CERR("Unable to copy config!");
    }
  }

  CLOG("Successfully copied files.");
  std::vector<std::string> targs;
  subprocessArgs.clear();
  subprocessArgs.push_back(targetExecutablePath.string());
  for(auto pt : filesToRemove)
    subprocessArgs.push_back(pt);
  CLOG("Starting "<<targetExecutablePath<<" and exiting...");
  startSubprocess = true;
  subprocessPath = targetExecutablePath.string();
  continueLoop = false;

  fsmtx.unlock();
  return 0;
}

//Debug signal handlers
#ifdef DEBUG
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
#ifdef DEBUG
  signal(SIGINT, ctrlchandler);
  signal(SIGTERM, killhandler);
#endif

  if(argc > 1)
  {
    bool first = false;
    for(int i=1; i < argc; i++)
    {
      const char* arg = argv[i];
      CLOG("Removing "<<arg<<"...");
      try
      {
        if(fs::exists(arg)) fs::remove(arg);
      }catch(...)
      {
        CERR("Unable to remove "<<arg<<"...");
      }
    }
  }

  loadRootPath(argv[0]);
  loadSysInfo();

  CLOG("Using platform ID " << CHARLIE_PLATFORM);

  // Check / start the port lock
  CLOG("Using port "<<sysInfo.lock_port<<" to lock...");
  try {
    io_service service;
    ip::tcp::endpoint ep(ip::address::from_string( "127.0.0.1" ), sysInfo.lock_port);
    ip::tcp::acceptor acceptor(service, ep);

    if (loadServerPubKey() != 0)
    {
      CERR("Server public key load unsuccessful, continuing anyway...");
    }

    if (loadConfigFile() != SUCCESS)
    {
      CLOG("Can't load config file, will use reasonable defaults...");
    } else
    {
      deserializeConfig();
    }

    if(validateConfig()){
      serializeConfig();
      saveConfig();
    }

    if (!identityLoaded)
    {
      if (loadIdentityToCrypto() != SUCCESS)
      {
        CERR("Can't load the identity to crypto! Regenerating identity...");
        generateIdentity();
        serializeConfig();
        saveConfig();
      } else {
        CLOG("Loaded identity to crypto.");
      }
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
      try {
        boost::this_thread::sleep( boost::posix_time::milliseconds(200) );
      } catch(int e)
      {
#ifdef DEBUG
        CLOG("Exception during sleep, assuming shutdown.");
        continueLoop = false;
#endif
      }
    }
    CLOG("Exiting...");
    FREE_OLD_CONFIG;
  }catch(std::exception& ex)
  {
    CERR("Error (probably already running), quitting.");
    CERR(ex.what());
  }
  return 0;
}
