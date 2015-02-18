#include <cserver/ModuleTable.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <ctime>
#include <charlie/hash.h>

int generateModuleTableFromJson(const char* json, char** output, Crypto* crypt, size_t* outputSize, bool doHash, std::string libprefix, std::string libsuffix, std::string rootPath, bool doSign)
{
  CLOG("Parsing json...");

  //Parse json
  rapidjson::Document d;
  if(d.Parse(json).HasParseError())
  {
    CERR("Unable to parse json input.");
    CLOG(json);
    return -1;
  }

  if(!d.IsObject())
  {
    CERR("Document must be an object.");
    return -1;
  }

  if(!(d.HasMember("modules") && d["modules"].IsArray()))
  {
    CERR("Document must have a modules array.");
    return -1;
  }

  if(d.HasMember("name") && d["name"].IsString())
    CLOG("Building CModuleTable \""<<d["name"].GetString()<<"\"...");

  charlie::CModuleTable table;
  table.set_timestamp(std::time(0));
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

    charlie::CModule* mod = table.add_modules();
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
    if(ix.HasMember("info") && ix["info"].IsObject() && ix.HasMember("info_type") && ix["info_type"].IsString())
    {
      std::string ityp (ix["info_type"].GetString());
      if(ityp.compare("CManagerInfo") == 0)
      {
        CLOG("info_type recognized CManagerInfo");
        modules::manager::CManagerInfo manInfo;
        const rapidjson::Value& initUrls = ix["info"]["init_urls"];
        for (rapidjson::SizeType oi = 0; oi < initUrls.Size(); oi++)
        {
          manInfo.add_init_url(initUrls[oi].GetString());
          CLOG("Adding init url: "<<initUrls[oi].GetString());
        }
        std::string* info = mod->mutable_info();
        if(!manInfo.SerializeToString(info))
        {
          CERR("Unable to serialize modInfo to string");
        }
        CLOG("has_info: "<<mod->has_info());
      }
    }
    if(doHash)
    {
      //gchar* filename = g_module_build_path((const gchar *) full_path->string().c_str(), name.c_str());
      std::string fns = rootPath+"/"+libprefix+name+libsuffix;
      const char* filename = fns.c_str();
      CLOG("filename: "<<filename);
      unsigned char* digest;
      if(sha256File(filename, &digest)!=0)
      {
        CERR("Unable to hash file for some reason.");
      }
      else
      {
        mod->set_hash(digest, SHA256_DIGEST_LENGTH);
        char mdString[SHA256_DIGEST_LENGTH*2+1];
        for (int i = 0; i < SHA256_DIGEST_LENGTH; i++)
          sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
        CLOG("digest: "<<mdString);
        free(digest);
      }
    }
  }

  std::string outd;
  if(!table.SerializeToString(&outd)){
    CERR("Unable to serialize module table to string.");
    return -1;
  }

  size_t outSize = outd.length();
  char* out = (char*)malloc(outSize*sizeof(char));
  memcpy(out, outd.c_str(), outd.length());

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
      out = (char*)malloc(sizeof(char)*outSize);
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
