#include <charlie/System.h>
#include <charlie/machine_id.h>
using namespace std;

System::System(void)
{
}

System::~System(void)
{
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

  exePath = full_path;
  CLOG("Path to exe:  " << exePath);
  rootPath = full_path.remove_filename()/"/";
  CLOG("Path to root: " << rootPath);
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
  const char* hostname = getMachineName();
  return SUCCESS;
}

int System::main(int argc, const char* argv[])
{
  loadRootPath(argv[0]);
  loadConfigFile();
}
