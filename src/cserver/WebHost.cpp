#include <cserver/System.h>
#include <cserver/WebHost.h>
#include <cserver/ModuleTable.h>
#include <charlie/xor.h>
#include <stdio.h>
#include <Logging.h>
#include <gmodule.h>
#include <iostream>
#include <fstream>
#include <charlie/base64.h>
#include <sstream>
#include <ctime>
#include <boost/filesystem.hpp>

#include <boost/algorithm/string/predicate.hpp>

#ifdef DEBUG
#include <openssl/md5.h>
#endif

#include <boost/thread/thread.hpp>

static int send_notfound(struct mg_connection *conn)
{
  CLOG(" -> 404 not found");
  mg_send_status(conn, 404);
  return MG_TRUE;
}

static int send_modtable(struct mg_connection *conn, WebHost* host)
{
  CLOG(" -> module table");
  mg_send_header(conn, "Content-Type", "text/plain");
  mg_printf_data(conn, "- %s", (char *) host->info.c_str());
  return MG_TRUE;
}

static int ev_handler(struct mg_connection *conn, enum mg_event ev)
{
  WebHost* host = (WebHost*)conn->server_param;

  if (ev == MG_REQUEST) {
    std::string uri = std::string(conn->uri);
    CLOG(conn->request_method<<" "<<conn->uri);

    if(uri.find("initt")!=std::string::npos)
      return send_modtable(conn, host);

    // Request to download a module
    if(boost::starts_with(uri, "/ereq/") && uri.length() > 6)
    {
      // Parse everything after ereq as base64
      std::string b64(uri.substr(6));
      unsigned char* buf;
      int len = base64Decode(b64.c_str(), b64.length(), &buf);

      // Parse the binary as a CRSABuffer
      charlie::CRSABuffer rbuf;
      if(!rbuf.ParseFromArray(buf, len))
      {
        CERR(" -> unable to parse RSABuffer");
        free(buf);
        return send_notfound(conn);
      }

      free(buf);

      // Decrypt the CSRABuffer
      len = decryptRsaBuf(&rbuf, host->sys->crypt, &buf);
      if(len == FAILURE)
      {
        CERR(" -> unable to decrypt request RSA.");
        free(buf);
        return send_notfound(conn);
      }

      // Try to parse the CDownloadRequest
      modules::manager::CDownloadRequest dreq;
      if(!dreq.ParseFromArray(buf, len))
      {
        CERR(" -> unable to decrypt CDownloadRequest");
        free(buf);
        return send_notfound(conn);
      }

      free(buf);

      if(!dreq.has_id())
      {
        CERR(" -> CDownloadRequest is invalid");
        return send_notfound(conn);
      }

      int id = dreq.id();
      u32 platform = dreq.platform();

      // Check the module ID in the known table
      int emcount = host->table->modules_size();
      charlie::CModule *emod = NULL;
      int i;
      for(i=0;i<emcount;i++)
      {
        emod = host->table->mutable_modules(i);
        if(emod->id() == id) break;
        emod = NULL;
      }

      if(emod == NULL)
      {
        CERR(" -> cannot find the module "<<id<<" requested.");
        return send_notfound(conn);
      }

      // select the binary
      charlie::CModuleBinary* bin = NULL;
      {
        int bincount = emod->binary_size();
        int i;
        for(i=0;i<bincount;i++)
        {
          bin = emod->mutable_binary(i);
          if(bin->platform() & platform) break;
          bin = NULL;
        }
      }

      if (bin == NULL)
      {
        CLOG(" -> unsupported platform " << platform);
        return send_notfound(conn);
      }

      // Build the filename
      std::ostringstream fss;
      fss << "./modules/";
      fss << platformToPrefix(platform);
      fss << emod->id();
      fss << platformToSuffix(platform);
      std::string fns = fss.str();
      fss.clear();
      fss.str("");

      // Check file exists
      if(!boost::filesystem::exists(fns))
      {
        CERR(" -> file "<<fns<<" doesn't exist.");
        return send_notfound(conn);
      }

      CLOG(" -> serving "<<fns);
      mg_send_file(conn, fns.c_str(), NULL);

      return MG_MORE;
    }

    return send_notfound(conn);
  } else if (ev == MG_AUTH) {
    return MG_TRUE;
  } else {
    return MG_FALSE;
  }
}


WebHost::WebHost(System* sys)
{
  this->sys = sys;
}

WebHost::~WebHost()
{
  if(this->server) mg_destroy_server(&this->server);
  if(this->table) delete this->table;
}


void WebHost::mainThread()
{
  std::ifstream inFile ("init.json", std::ios_base::in);

  if(inFile.is_open())
  {
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    inFile.close();

    try
    {
      table = generateModuleTableFromJson2(buffer.str().c_str(), sys->crypt, std::string("./modules"));
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
#ifdef DEBUG
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
    // Run the server
    this->server = mg_create_server((void*)this, ev_handler);
    mg_set_option(this->server, "listening_port", "9921");

    CLOG("Web listening on localhost:9921");
    for (;;) mg_poll_server((struct mg_server *) this->server, 1000);
  }catch (std::exception &e) {
    CERR(e.what());
  }
}
