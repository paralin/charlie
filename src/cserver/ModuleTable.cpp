#include <cserver/ModuleTable.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <ctime>
#include <charlie/hash.h>
#include <iostream>
#include <string>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>

namespace fs = boost::filesystem;

charlie::CModuleTable* generateModuleTableFromJson2(const char* json, Crypto* crypt, std::string rootPath)
{
  CLOG("Parsing json...");

  //Parse json
  rapidjson::Document d;
  if(d.Parse(json).HasParseError())
  {
    CERR("Unable to parse json input.");
    CLOG(json);
    throw std::runtime_error("1");
  }

  if(!d.IsObject())
  {
    CERR("Document must be an object.");
    throw std::runtime_error("2");
  }

  if(!(d.HasMember("modules") && d["modules"].IsArray()))
  {
    CERR("Document must have a modules array.");
    throw std::runtime_error("3");
  }

  if(d.HasMember("name") && d["name"].IsString())
    CLOG("Building CModuleTable \""<<d["name"].GetString()<<"\"...");

  charlie::CModuleTable* table = new charlie::CModuleTable();
  table->set_timestamp(std::time(0));
  const rapidjson::Value& modules = d["modules"];
  for (rapidjson::SizeType i = 0; i < modules.Size(); i++)
  {
    const rapidjson::Value& ix = modules[i];
    if(!ix.IsObject())
    {
      CERR("modules["<<i<<"] must be an object.");
      continue;
    }
    if(!ix.HasMember("name") || !ix["name"].IsString())
    {
      CERR("modules["<<i<<"].id must be a number");
      continue;
    }

    charlie::CModule* mod = table->add_modules();
    //calculate the ID
    std::string name(ix["name"].GetString());
    CLOG("== "<<name<<" ==");
    unsigned int id = hashString(name.c_str());
    mod->set_id(id);
    CLOG("id:   "<<id);
    if(ix.HasMember("mainfcn") && ix["mainfcn"].IsBool())
    {
      mod->set_mainfcn(ix["mainfcn"].GetBool());
    }
    if(ix.HasMember("initial") && ix["initial"].IsBool())
    {
      mod->set_initial(ix["initial"].GetBool());
    }
    if (ix.HasMember("priority") && ix["priority"].IsNumber())
    {
      mod->set_priority(ix["priority"].GetInt());
    }
    if (ix.HasMember("capabilities") && ix["capabilities"].IsNumber())
    {
      mod->set_capabilities(ix["capabilities"].GetInt());
      CLOG("capabilities: " << mod->capabilities());
    }
    if (ix.HasMember("binary") && ix["binary"].IsArray())
    {
      const rapidjson::Value& binaries = ix["binary"];
      for (rapidjson::SizeType oi = 0; oi < binaries.Size(); oi++)
      {
        const rapidjson::Value& bind = binaries[oi];
        u32 platformMask = bind["platform"].GetInt();
        for (u32 platform = charlie::ESystemPlatform_MIN; platform <= charlie::ESystemPlatform_MAX; platform++)
        {
          if (!charlie::ESystemPlatform_IsValid((int) platform) || !(platformMask & platform))
            continue;

          charlie::CModuleBinary* bin = mod->add_binary();
          bin->set_platform(platform);

          // Filename of library
          fs::path fns;
          {
            std::ostringstream fss;
            fss << rootPath;
            fss << "/";
            fss << platformToPrefix(bin->platform());
            fss << mod->id();
            fss << platformToSuffix(bin->platform());
            fns = fs::path(fss.str());
          }
          if (!fs::exists(fns))
          {
            CLOG("Expected path: " << fns.string());
            CERR("Unable to find binary " << fns.filename() << ", excluding.");
            mod->mutable_binary()->RemoveLast();
            continue;
          }
          if (bind.HasMember("acquire") && bind["acquire"].IsArray())
          {
            const rapidjson::Value& acquires = bind["acquire"];
            for(rapidjson::SizeType oi = 0; oi < acquires.Size(); oi++)
            {
              charlie::CModuleAcquire* acq = bin->add_acquire();
              int acqti = (int)acquires[oi]["type"].GetInt();
              switch(acqti)
              {
                case (int)charlie::HTTP_GET:
                  {
                    std::string httpg = acquires[oi]["data"].GetString();
                    CLOG("Adding HTTPGET acquire to "<<httpg<<" platform "<<bin->platform()<<"...");
                    acq->set_type((charlie::CAcquireType)acqti);
                    acq->set_data(httpg);
                    break;
                  }
                case (int)charlie::HTTP_SIGNED:
                  {
                    std::string httpg = acquires[oi]["data"].GetString();
                    CLOG("Adding HTTPSIGNED acquire to "<<httpg<<" platform "<<bin->platform()<<"...");
                    acq->set_type((charlie::CAcquireType)acqti);
                    acq->set_data(httpg);
                    break;
                  }
                default:
                  {
                    CLOG("Unknown acquire method: "<<acqti);
                    bin->mutable_acquire()->RemoveLast();
                    break;
                  }
              }
            }
          }
          // Make a new scope
          {
            const char* filename = fns.c_str();
            CLOG("filename: "<<filename);
            unsigned char* digest;
            if(sha256File(filename, &digest)!=0)
            {
              CERR("Unable to hash file for some reason.");
            }
            else
            {
              bin->set_hash(digest, SHA256_DIGEST_LENGTH);
              char mdString[SHA256_DIGEST_LENGTH*2+1];
              for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
                sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
              CLOG("digest: "<<mdString);
              free(digest);
            }
          }
        }
      }
    }
    /*
       if(ix.HasMember("acquire") && ix["acquire"].IsArray())
       {
       }
       */
    if(ix.HasMember("info") && ix["info"].IsObject() && ix.HasMember("info_type") && ix["info_type"].IsString())
    {
      std::string ityp (ix["info_type"].GetString());
      if(ityp.compare("CManagerInfo") == 0)
      {
        CLOG("info_type recognized CManagerInfo");
        modules::manager::CManagerInfo manInfo;

        {
          const rapidjson::Value& initUrls = ix["info"]["init_urls"];
          for (rapidjson::SizeType oi = 0; oi < initUrls.Size(); oi++)
          {
            manInfo.add_init_url(initUrls[oi].GetString());
            CLOG("Adding init url: "<<initUrls[oi].GetString());
          }
        }

        {
          const rapidjson::Value& serverRoots = ix["info"]["server_roots"];
          for (rapidjson::SizeType oi = 0; oi < serverRoots.Size(); oi++)
          {
            manInfo.add_server_root(serverRoots[oi].GetString());
            CLOG("Adding server root: "<<serverRoots[oi].GetString());
          }
        }

        {
          const rapidjson::Value& serverKeys = ix["info"]["server_keys"];
          for (rapidjson::SizeType oi = 0; oi < serverKeys.Size(); oi++)
          {
            manInfo.add_server_key(serverKeys[oi].GetString());
            CLOG("Adding server key: "<<serverKeys[oi].GetString());
          }
        }

        std::string* info = mod->mutable_info();
        if(!manInfo.SerializeToString(info))
        {
          CERR("Unable to serialize modInfo to string");
        }
        CLOG("has_info: "<<mod->has_info());
      }
      else if(ityp.compare("CClientInfo") == 0)
      {
        CLOG("info_type recognized CClientInfo");
        modules::client::CClientInfo directConnectInfo;

        {
          const rapidjson::Value& serverAddrs = ix["info"]["server_addr"];
          for (rapidjson::SizeType oi = 0; oi < serverAddrs.Size(); oi++)
          {
            directConnectInfo.add_server_addr(serverAddrs[oi].GetString());
            CLOG("Adding server addr: "<<serverAddrs[oi].GetString());
          }
        }

        std::string* info = mod->mutable_info();
        if(!directConnectInfo.SerializeToString(info))
        {
          CERR("Unable to serialize modInfo to string");
        }
        CLOG("has_info: "<<mod->has_info());
      }
    }
  }
  return table;
}

int generateModuleTableFromJson(const char* json, unsigned char** output, Crypto* crypt, size_t* outputSize, std::string rootPath, bool doSign)
{
  charlie::CModuleTable* table;
  try {
    table = generateModuleTableFromJson2(json, crypt, rootPath);
  }catch(...)
  {
    CERR("Unable to generate module table from json.");
    return -1;
  }

  std::string outd;
  if(!table->SerializeToString(&outd)){
    CERR("Unable to serialize module table to string.");
    delete table;
    return -1;
  }

  size_t outSize = outd.length();
  unsigned char* out = (unsigned char*)malloc(outSize*sizeof(unsigned char));
  memcpy(out, outd.c_str(), outd.length());

  delete table;

  if(doSign)
  {
    if(crypt != NULL)
    {
      unsigned char* sig;
      size_t sigLen = (size_t)crypt->digestSign((const unsigned char*)out, outSize, &sig, false);
      if(sigLen == FAILURE)
      {
        CERR("Unable to sign the table.");
        return -1;
      }
      charlie::CSignedBuffer rbuf;
      rbuf.set_data(out, outSize);
      rbuf.set_sig(sig, sigLen);
      free(sig);
      free(out);
      CLOG("Signed the module table.");
      outSize = rbuf.ByteSize();
      out = (unsigned char*)malloc(sizeof(unsigned char)*outSize);
      *output = out;
      if(!rbuf.SerializeToArray(out, outSize))
      {
        CERR("Unable to serialize signature buffer to array.");
        free(out);
        return -1;
      }
    }
    else
    {
      CERR("Unable to load crypto to encrypt modtable! Refusing to continue...");
      return -1;
    }
  }
  *outputSize = outSize;
  return 0;
}

const char* platformToPrefix(u32 platform)
{
  if (platform & charlie::LINUX || platform & charlie::MAC)
    return "lib";
  return "";
}

const char* platformToSuffix(u32 platform)
{
  if (platform & charlie::WINDOWS)
    return ".dll";

  return ".so";
}
