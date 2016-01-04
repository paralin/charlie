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
  time_t now = time(NULL);
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

    charlie::CModule* mod = new charlie::CModule();
    mod->set_timestamp(now);
    //calculate the ID
    std::string name(ix["name"].GetString());
    CLOG("== "<<name<<" ==");
    unsigned int id = hashString(name.c_str());
    mod->set_id(id);
    CLOG("id:   "<<id);

    if(ix.HasMember("mainfcn") && ix["mainfcn"].IsBool())
      mod->set_mainfcn(ix["mainfcn"].GetBool());

    if(ix.HasMember("initial") && ix["initial"].IsBool())
      mod->set_initial(ix["initial"].GetBool());

    if (ix.HasMember("priority") && ix["priority"].IsNumber())
      mod->set_priority(ix["priority"].GetInt());

    if (ix.HasMember("capabilities") && ix["capabilities"].IsNumber())
    {
      mod->set_capabilities(ix["capabilities"].GetInt());
      CLOG("capabilities: " << mod->capabilities());
    }

    if (ix.HasMember("bind_lazy") && ix["bind_lazy"].IsBool())
      mod->set_bind_lazy(ix["bind_lazy"].GetBool());

    if (ix.HasMember("bind_local") && ix["bind_local"].IsBool())
      mod->set_bind_local(ix["bind_local"].GetBool());

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
            const char* filename = fns.string().c_str();
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
            std::string r(initUrls[oi].GetString());
#ifdef IS_CHARLIE_RELEASE
            if (r.find("localhost") != std::string::npos)
            {
              CLOG("Skipping local root " << r);
              continue;
            }
#endif
            manInfo.add_init_url(r);
            CLOG("Adding init url: "<<r);
          }
        }

        {
          const rapidjson::Value& serverRoots = ix["info"]["server_roots"];
          for (rapidjson::SizeType oi = 0; oi < serverRoots.Size(); oi++)
          {
            std::string r(serverRoots[oi].GetString());
#ifdef IS_CHARLIE_RELEASE
            if (r.find("localhost") != std::string::npos)
            {
              CLOG("Skipping local root " << r);
              continue;
            }
#endif
            manInfo.add_server_root(r);
            CLOG("Adding server root: "<<r);
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
            std::string r(serverAddrs[oi].GetString());
#ifdef IS_CHARLIE_RELEASE
            if (r.find("localhost") != std::string::npos)
            {
              CLOG("Skipping local root " << r);
              continue;
            }
#endif
            directConnectInfo.add_server_addr(r);
            CLOG("Adding server addr: "<<r);
          }
        }

        std::string* info = mod->mutable_info();
        if(!directConnectInfo.SerializeToString(info))
        {
          CERR("Unable to serialize modInfo to string");
        }
        CLOG("has_info: "<<mod->has_info());
      }
      else if(ityp.compare("CTormInfo") == 0)
      {
        CLOG("info_type recognized CTormInfo");
        modules::torm::CTormInfo torInfo;

        {
          const rapidjson::Value& endpoints = ix["info"]["endpoints"];
          for (rapidjson::SizeType oi = 0; oi < endpoints.Size(); oi++)
          {
            torInfo.add_endpoints(endpoints[oi].GetString());
            CLOG("Adding tor endpoint: "<<endpoints[oi].GetString());
          }
        }

        std::string* info = mod->mutable_info();
        if(!torInfo.SerializeToString(info))
        {
          CERR("Unable to serialize modInfo to string");
        }
        CLOG("has_info: "<<mod->has_info());
      }
    }

    // Sign it
    charlie::CSignedBuffer* buf = table->add_signed_modules();
    buf->set_data(mod->SerializeAsString());
    delete mod;
    if (updateSignedBuf(buf, crypt) != SUCCESS)
    {
      CERR("Error updating module signed buffer.");
    }
  }
  return table;
}

inline const char* platformToPrefix(u32 platform)
{
  /*
  if (platform & charlie::LINUX || platform & charlie::MAC)
    return "lib";
  */
  return "lib";
}

const char* platformToSuffix(u32 platform)
{
  if (platform & charlie::WINDOWS)
    return ".dll";

  return ".so";
}
