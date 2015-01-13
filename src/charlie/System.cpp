#include <charlie/System.h>
#include <charlie/machine_id.h>
#include <charlie/ServerKey_Data.h>

using namespace std;

System::System(void)
{
  crypto = new Crypto();
  mManager = new ModuleManager();
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
  char * cexePath = new char [exePath.length()+1];
  std::strcpy (cexePath, exePath.c_str());
  sysInfo.exe_path = (const char*)cexePath;
  CLOG("Path to exe:  " << sysInfo.exe_path);
  std::string rootPath = (full_path.remove_filename()/"/").string();
  char * crootPath = new char [rootPath.length()+1];
  std::strcpy (crootPath, rootPath.c_str());
  sysInfo.root_path = (const char*)crootPath;
  CLOG("Path to root: " << sysInfo.root_path);
}

int testEncryption() {
  charlie::CMsgHeader header;
  header.set_timestamp(50);

  Crypto crypto;
  Crypto crypto2;

  unsigned char *c1pub = NULL;
  unsigned char *c2pub = NULL;

  int c1len = crypto.getLocalPubKey(&c1pub);
  int c2len = crypto2.getLocalPubKey(&c2pub);

  crypto.setRemotePubKey(c2pub, c2len);
  crypto2.setRemotePubKey(c1pub, c1len);

  string msg;
  unsigned char *encMsg = NULL;
  char *decMsg          = NULL;
  int encMsgLen;
  int decMsgLen;

  unsigned char *ek;
  unsigned char *iv;
  size_t ekl;
  size_t ivl;

  while(!cin.eof()) {
    // Get the message to encrypt
    printf("Message to RSA encrypt: ");
    fflush(stdout);
    getline(cin, msg);

    // Encrypt the message with RSA
    // Note the +1 tacked on to the string length argument. We want to encrypt the NUL terminator too. If we don't,
    // we would have to put it back after decryption, but it's easier to keep it with the string.
    if((encMsgLen = crypto.rsaEncrypt((const unsigned char*)msg.c_str(), msg.size()+1, &encMsg, &ek, &ekl, &iv, &ivl)) == -1) {
      fprintf(stderr, "Encryption failed\n");
      return 1;
    }

    // Print the encrypted message as a base64 string
    char* b64String = base64Encode(encMsg, encMsgLen);
    printf("Encrypted message: %s\n", b64String);

    apply_xor(b64String, strlen(b64String), "what's up", strlen("what's up"));

    printf("XOR applied %s\n", b64String);

    apply_xor(b64String, strlen(b64String), "what's up", strlen("what's up"));

    // Decrypt the message with the second one
    if((decMsgLen = crypto2.rsaDecrypt(encMsg, (size_t)encMsgLen, ek, ekl, iv, ivl, (unsigned char**)&decMsg)) == -1) {
      fprintf(stderr, "Decryption failed\n");
      return 1;
    }
    printf("Decrypted message: %s\n", decMsg);

    // No one likes memory leaks
    free(encMsg);
    free(decMsg);
    free(ek);
    free(iv);
    free(b64String);
    encMsg    = NULL;
    decMsg    = NULL;
    ek        = NULL;
    iv        = NULL;
    b64String = NULL;
  }

  return 0;
}

int System::loadConfigFile()
{
  CLOG("Loading config file "<<sysInfo.config_filename<<".");
  if(!boost::filesystem::exists(sysInfo.config_filename))
    return -1;
  ifstream configFile;
  streampos size;
  configFile.open (sysInfo.config_filename, ios::in|ios::binary|ios::ate);
  if(configFile.is_open()){
    size = configFile.tellg();
    if(configDataSize != 0) {free(configData);configDataSize=0;}
    configData = new char[size];
    configFile.seekg(0, ios::beg);
    configFile.read(configData, size);
    configFile.close();
    CLOG("Loaded config file, "<<size<<" length.");
    apply_xor(configData, size, sysInfo.system_id, strlen(sysInfo.system_id));
    configDataSize = (int)size;
    return SUCCESS;
  } else
    return -1;
}

void System::loadDefaultConfig()
{
  CLOG("Loading default config...");
  validateConfig();
  serializeConfig();
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

void System::validateConfig()
{
  CLOG("Validating config...");
  if(!config.has_identity() || !config.identity().has_private_key() || !config.identity().has_public_key())
  {
    generateIdentity();
  }
  CLOG("Config validation complete.");
}

void System::serializeConfig()
{
  if(configDataSize != 0) {free(configData);configDataSize=0;}
  configDataSize = config.ByteSize();
  configData = (char*)malloc(sizeof(char)*configDataSize);
  if(!config.SerializeToArray(configData, configDataSize))
    CERR("Unable to serialize config to array.");
}

void System::deserializeConfig()
{
  if(!config.ParseFromArray(configData, configDataSize))
    CERR("Unable to parse config from array.");
}

void System::saveConfig()
{
  if(configDataSize == 0)
  {
    CERR("Not saving an empty configData buffer.");
    return;
  }
  char* toSave = (char*)malloc(sizeof(char)*configDataSize);
  strncpy(toSave, configData, configDataSize);
  apply_xor(toSave, configDataSize, sysInfo.system_id, strlen(sysInfo.system_id));
  CLOG("Saving config file to "<<sysInfo.config_filename);
  ofstream configFile (sysInfo.config_filename, ios::out|ios::binary);
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
  CLOG("ESystemID: "<<sysInfo.b64_system_id);
  sysInfo.cpu_hash = getCpuHash();
  CLOG("CPU Hash: "<<sysInfo.cpu_hash);
  u16 filenameLen = sysInfo.cpu_hash%10;
  if(filenameLen < 4) filenameLen = 4;
  std::string filename (sysInfo.b64_system_id);
  filename = filename.substr(0, filenameLen);
  CLOG("Filename: "<<filename);
  char * cstr = new char [filename.length()+1];
  std::strcpy (cstr, filename.c_str());
  sysInfo.config_filename = (const char*)cstr;
}

int System::loadIdentityToCrypto()
{
  const std::string pkey = config.identity().private_key();
  char * cstr = new char [pkey.length()+1];
  std::strcpy (cstr, pkey.c_str());
  int r1 = crypto->setLocalPriKey((unsigned char*)cstr, pkey.length()+1);
  free(cstr);

  const std::string pubkey = config.identity().public_key();
  char * pcstr = new char [pubkey.length()+1];
  std::strcpy (pcstr, pubkey.c_str());
  int r2 = crypto->setLocalPubKey((unsigned char*)pcstr, pubkey.length()+1);
  free(pcstr);

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
    crypto->setRemotePubKey((unsigned char*)pub_key, (size_t)pub_key_len);
    CLOG("===== SERVER KEY =====");
    CLOG(pub_key);
    CLOG("======================");
    free(pub_key);
    return 0;
  }
}

int System::main(int argc, const char* argv[])
{
  loadRootPath(argv[0]);
  loadSysInfo();
  if(loadConfigFile() != SUCCESS)
  {
    CLOG("Can't load config file, building new config...");
    loadDefaultConfig();
    saveConfig();
  }else
  {
    deserializeConfig();
    validateConfig();
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
  if(loadServerPubKey() != 0)
  {
    CERR("Server public key load unsuccessful, continuing anyway...");
  }
  return 0;
}
