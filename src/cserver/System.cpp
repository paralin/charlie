#include <cserver/System.h>
#include <charlie/xor.h>
#include <charlie/base64.h>
#include <sstream>
#include <ctime>
#include <fstream>
#include <boost/thread.hpp>
#include <cserver/ModuleTable.h>
#include <boost/filesystem.hpp>

#define RFAIL(fcn, msg) res=fcn;if(res!=0){CERR(msg);return res;}

System::System()
{
  crypt = new Crypto();
  host = new WebHost(this);
  nHost = new NetHost(this);
}

System::~System()
{
  delete host;
  delete crypt;
  delete nHost;
}

charlie::CModuleTable* System::generateModuleTableFromFile(const char* path)
{
  std::ifstream inFile (path, std::ios_base::in);
  if (!inFile.is_open())
  {
    CERR("Unable to find path " << path);
    return NULL;
  }

  std::stringstream buffer;
  buffer << inFile.rdbuf();
  inFile.close();

  return generateModuleTableFromJson2(buffer.str().c_str(), crypt, std::string("./modules"));
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
    privateKey = pkey;
    if (crypt->setLocalPriKey((unsigned char*)pkey.c_str(), pkey.length()) != SUCCESS)
    {
      CERR("Private key is invalid.");
      delete[] memblock;
      return 3;
    }
    delete[] memblock;
  }
  else
  {
    CERR("Can't open file server_identity...");
    return -1;
  }

  return 0;
}

void System::webTh()
{
  host->mainThread();
}

std::vector<std::shared_ptr<ServerModuleInstance>> System::buildModuleSet(CharlieClient* client)
{
  std::vector<std::shared_ptr<ServerModuleInstance>> mods;

  // Iterate over files in the server_modules dir and load them
  boost::filesystem::path p("./server_modules/");
  boost::filesystem::directory_iterator end;
  for (boost::filesystem::directory_iterator itr(p); itr != end; ++itr)
    if (boost::filesystem::is_regular_file(itr->path()))
      mods.emplace_back(new ServerModuleInstance(itr->path().string(), client));

  return mods;
}

int System::main(int argc, const char* argv[])
{
  int res;
  RFAIL(loadCrypto(), "Unable to load crypto!");

  boost::thread webThread(boost::bind(&System::webTh, this));
  nHost->mainThread();

  CLOG("Exiting...");
  return 0;
};

