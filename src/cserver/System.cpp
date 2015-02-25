#include <Common.h>
#include <cserver/System.h>
#include <cserver/ModuleTable.h>
#include <charlie/xor.h>
#include <stdio.h>
#include <gmodule.h>
#include <iostream>
#include <fstream>
#include <charlie/base64.h>
#include <sstream>
#include <boost/network/protocol/http/server.hpp>

#ifndef NDEBUG
#include <openssl/md5.h>
#endif

#define RFAIL(fcn, msg) res=fcn;if(res!=0){CERR(msg);return res;}
#ifdef WIN32
#define G_MODULE_PREFIX
#else
#define G_MODULE_PREFIX "lib"
#endif

namespace http = boost::network::http;

System::System()
{
  crypt = new Crypto();
}

System::~System()
{
  delete crypt;
}

struct hello_world;
typedef http::server<hello_world> server;

struct hello_world {
  std::string info;
  void log(std::string const& str)
  {
    CLOG(str);
  };
  void operator() (server::request const &request, server::response &response) {
    std::string ip = source(request);
    response = server::response::stock_reply(
        server::response::ok, std::string("Hello, ") + ip + "!" + "\n\n" + info);
  };
};

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

  std::ifstream inFile ("init.json", std::ios_base::in);
  hello_world handler;
  if(inFile.is_open())
  {
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();

    try{
      charlie::CModuleTable* table = generateModuleTableFromJson2(buffer.str().c_str(), crypt, std::string(G_MODULE_PREFIX), std::string(".")+std::string(G_MODULE_SUFFIX), std::string("./modules/linux"), true);
      charlie::CWebInformation info;
      info.set_allocated_mod_table(table);
      size_t outSize = info.ByteSize();
      unsigned char* out = (unsigned char*)malloc(outSize*sizeof(unsigned char));
      if(info.SerializeToArray(out, outSize))
      {
        unsigned char* sig;
        size_t sigLen = (size_t)crypt->digestSign((const unsigned char*)out, outSize, &sig, false);
        if(sigLen == FAILURE)
        {
          CERR("Unable to sign the table.");
        }else
        {
          charlie::CSignedBuffer buf;
          buf.set_data(out, outSize);
          buf.set_sig(sig, sigLen);
          free(out);
          free(sig);
          outSize = buf.ByteSize();
          out = (unsigned char*)malloc(sizeof(unsigned char)*outSize);
          if(!buf.SerializeToArray(out, outSize))
          {
            CERR("Unable to serialize the signature to a array.");
          }
          else
          {
#ifndef NDEBUG
            {
              char mdString[33];
              unsigned char digest[MD5_DIGEST_LENGTH];
              MD5((const unsigned char*)out, outSize, (unsigned char*)&digest);
              for (int i = 0; i < 16; i++)
                sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
              CLOG("MD5 of data: "<<mdString);
              CLOG("Length of data: "<<outSize);
            }
#endif
            apply_xor(out, outSize, ONLINE_MTABLE_KEY, strlen(ONLINE_MTABLE_KEY));
            char* b64o = base64Encode((const unsigned char*)out, outSize);
            handler.info = std::string("@")+std::string(b64o);
            free(b64o);
            CLOG(handler.info);
          }
        }
        free(out);
      }
      else
      {
        CERR("Unable to serialize web info to array");
      }
    }catch(...)
    {
      CERR("Unable to generate module table.");
    }
  }else
  {
    CERR("Can't find test modtable json");
  }
  try {
    server::options options(handler);
    server server_(options.address("127.0.0.1").port("9921"));
    server_.run();
  }catch (std::exception &e) {
    CERR(e.what());
  }
  CLOG("Exiting...");
}

