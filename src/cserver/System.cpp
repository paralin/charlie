#include <cserver/System.h>
#include <charlie/xor.h>
#include <charlie/base64.h>
#include <sstream>
#include <ctime>
#include <fstream>

#define RFAIL(fcn, msg) res=fcn;if(res!=0){CERR(msg);return res;}

System::System()
{
  crypt = new Crypto();
  host = new WebHost(this);
}

System::~System()
{
  delete host;
  delete crypt;
}

int System::loadCrypto()
{
  std::ifstream inFile ("server_identity", std::ios_base::in|std::ios_base::binary|std::ios_base::ate);
  if(inFile.is_open())
  {
    std::streampos size;
    size = inFile.tellg();
    unsigned char* memblock;
    memblock = new unsigned char [size];
    inFile.seekg (0, std::ios_base::beg);
    inFile.read ((char*)memblock, size);
    inFile.close();

    CLOG("Loaded identity file...");

    CLOG("Applying XOR key \"serveridentity\"...");
    apply_xor(memblock, size, "serveridentity", strlen("serveridentity"));

    charlie::CIdentity ident;
    if(!ident.ParseFromArray(memblock, size))
    {
      CLOG("Unable to parse the identity.");
      delete[] memblock;
      return 2;
    }

    if(!ident.has_private_key())
    {
      CERR("Identity doesn't have a private key.");
      delete[] memblock;
      return 4;
    }

    std::string pkey = ident.private_key();
    if(!crypt->setLocalPriKey((unsigned char*)pkey.c_str(), pkey.length()) == SUCCESS)
    {
      CERR("Private key is invalid.");
      delete[] memblock;
      return 3;
    }
    delete[] memblock;
  }
  else
  {
    CERR("Can't open file "<<inFile<<"...");
    return -1;
  }

  return 0;
}

int System::main(int argc, const char* argv[])
{
  int res;
  RFAIL(loadCrypto(), "Unable to load crypto!");

  host->mainThread();

  CLOG("Exiting...");
}

