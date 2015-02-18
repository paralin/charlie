#include <cserver/System.h>
#include <cserver/ModuleTable.h>
#include <http/server.h>
#include <stdio.h>
#include <gmodule.h>
#include <iostream>
#include <fstream>
#include <charlie/base64.h>

#define RFAIL(fcn, msg) res=fcn;if(res!=0){CERR(msg);return res;}
#ifdef WIN32
#define G_MODULE_PREFIX
#else
#define G_MODULE_PREFIX "lib"
#endif

System::System()
{
  crypt = new Crypto();
}

System::~System()
{
  delete crypt;
}

int System::loadCrypto()
{
  std::ifstream inFile ("server_identity", std::ios_base::in|std::ios_base::binary|std::ios_base::ate);
  if(inFile.is_open())
  {
    std::streampos size;
    char* memblock;
    size = inFile.tellg();
    memblock = new char [size];
    inFile.seekg (0, std::ios_base::beg);
    inFile.read (memblock, size);
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

  std::ifstream inFile ("init.json", std::ios_base::in);
  if(inFile.is_open())
  {
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();

    char* output;
    size_t outputSize;
    if(generateModuleTableFromJson(buffer.str().c_str(), &output, crypt, &outputSize, true, std::string(G_MODULE_PREFIX), std::string(".")+std::string(G_MODULE_SUFFIX), std::string("./modules/linux"), true) == 0)
    {
      char* b64o = base64Encode((const unsigned char*)output, outputSize);
      CLOG(b64o);
      free(b64o);
    }
    else
    {
      CERR("Unable to generate module table");
    }
    free(output);
  }else
  {
    CERR("Can't find test modtable json");
  }
  CLOG("Exiting...");
}

