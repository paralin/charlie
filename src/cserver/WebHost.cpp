#include <cserver/System.h>
#include <cserver/WebHost.h>
#include <cserver/ModuleTable.h>
#include <charlie/xor.h>
#include <stdio.h>
#include <gmodule.h>
#include <iostream>
#include <fstream>
#include <charlie/base64.h>
#include <sstream>
#include <ctime>

#ifndef NDEBUG
#include <openssl/md5.h>
#endif

#include <boost/thread/thread.hpp>

namespace http = boost::network::http;

struct server_def {
  WebHost* web;
  void log(std::string const& str)
  {
    CLOG(str);
  };
  void operator() (server::request const &request, server::response &response) {
    web->processRequest(request, response);
  };
};

WebHost::WebHost(System* sys)
{
  this->sys = sys;
}

WebHost::~WebHost()
{
}

void WebHost::processRequest(server::request const &request, server::response &response)
{
  CLOG(request.method<<" "<<request.destination);
  response = server::response::stock_reply(server::response::ok, info);
  CLOG("... responded with init table");
}

void WebHost::mainThread()
{
  std::ifstream inFile ("init.json", std::ios_base::in);
  server_def handler;
  handler.web = this;

  if(inFile.is_open())
  {
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();

    try
    {
      charlie::CModuleTable* table = generateModuleTableFromJson2(buffer.str().c_str(), sys->crypt, std::string(G_MODULE_PREFIX), std::string(".")+std::string(G_MODULE_SUFFIX), std::string("./modules/linux"), true);
      charlie::CWebInformation info;
      info.set_timestamp(std::time(0));

      charlie::CSignedBuffer* mbuf = new charlie::CSignedBuffer();
      table->SerializeToString(mbuf->mutable_data());
      if(updateSignedBuf(mbuf, sys->crypt) == SUCCESS)
      {
        info.set_allocated_mod_table(mbuf);

        charlie::CSignedBuffer buf;
        info.SerializeToString(buf.mutable_data());
        if(updateSignedBuf(&buf, sys->crypt) == SUCCESS)
        {
          size_t outSize = buf.ByteSize();
          unsigned char* out = (unsigned char*)malloc(sizeof(unsigned char)*outSize);
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
            this->info = std::string("@")+std::string(b64o);
            free(b64o);
            CLOG(this->info);
            free(out);
          }
        }else{
          CERR("Can't sign web info.");
        }
      }else{
        CERR("Can't sign module table.");
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
    server server_(options
        .address("127.0.0.1")
        .port("9921"));
    server_.run();
  }catch (std::exception &e) {
    CERR(e.what());
  }
}
