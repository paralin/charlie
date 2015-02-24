//xxx: Later maybe this can be automated?
#define MODULE_ID 3133916783
#include <modules/manager/Manager.h>
#include <boost/thread.hpp>
#include <boost/regex.hpp>
#include <stdexcept>
#include <algorithm>
#include <openssl/md5.h>
#include <charlie/base64.h>

using namespace modules::manager;
using namespace boost::network;

ManagerModule::ManagerModule()
{
  MLOG("Manager module constructed...");
  pInter = new ManagerInter(this);
}

void ManagerModule::shutdown()
{
  MLOG("Shutting down manager module..");
  delete pInter;
}

void ManagerModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;
}

void ManagerModule::injectDependency(u32 id, void* dep)
{
  MLOG("Dep injected "<<id);
}

void ManagerModule::releaseDependency(u32 id)
{
  MLOG("Dep released "<<id);
}

std::string ManagerModule::fetchOnionCabUrl(const std::string& url)
{
  bool hasRehashed = false;

  /*
   * Send the request, if we get a tttt= rehash
   * If we have already rehashed and get ttt= again exit
   */
  while(true)
  {
    http::client::request request(url);
    if(onionCabHash.empty() && stor.has_onion_cab_cookie())
    {
      onionCabHash = std::string(stor.onion_cab_cookie());
      MLOG("Loaded onion.cab cookie from storage: "<<onionCabHash);
    }
    std::string cookie;
    if(!onionCabHash.empty())
    {
      cookie = std::string("onion_cab_iKnowShit=")+onionCabHash+";";
      CLOG("Using header: Cookie: "<<cookie);
      request << header("Cookie", cookie.c_str());
    }
    request << header("Accept", "text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
    request << header("Accept-Encoding", "identity");
    request << header("Accept-Language", "en-US,en;q=0.8,ru;q=0.6");
    request << header("User-Agent", "Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/40.0.2214.111 Safari/537.36");
    request << header("Connection", "close");
    http::client::response response = client.get(request);
    const std::string s = body(response);

    if(s.find("loadAgreement()")!=std::string::npos)
    {
      if(hasRehashed)
      {
        MERR("Onion cab still not accepting our verification hash.");
        throw std::runtime_error("onion_cab");
      }
      hasRehashed = true;
      boost::match_results<std::string::const_iterator> results;

      //Find the  value
      boost::regex t5r("(var ttttt=)('.*?')");
      boost::regex tk3r("(\"tktktk\\|atob\\|)([a-zA-Z0-9]*)(\".split\\(\"\\|\"\\))");
      if (boost::regex_search(s.begin(), s.end(), results, t5r))
      {
        std::string t5(results[2]);
        t5 = t5.substr(1, t5.length()-2);
        MLOG("t5 value: "<<t5);
        //Fetch tk3 value
        request = http::client::request("https://onion.cab/js/jQuery.js");
        request << header("Connection", "close");
        response = client.get(request);
        const std::string jqs = body(response);

        //Find tk3
        if (boost::regex_search(jqs.begin(), jqs.end(), results, tk3r))
        {
          std::string tk3(results[2]);
          //base 64 decode
          {
            unsigned char* b64d;
            size_t b64dl = (size_t)base64Decode(tk3.c_str(), tk3.length(), &b64d);
            tk3 = std::string((const char*)b64d, b64dl);
            free(b64d);
          }

          MLOG("tk3 value: "<<tk3);
          //Now we have to md5...
          char mdString[33];
          {
            std::string tksum = tk3+t5;
            unsigned char digest[MD5_DIGEST_LENGTH];
            MD5((const unsigned char*)tksum.c_str(), tksum.length(), (unsigned char*)&digest);
            for (int i = 0; i < 16; i++)
              sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
          }
          MLOG("Final onionCab iKnowShit is "<<mdString);
          onionCabHash = std::string(mdString, 33);
          stor.set_onion_cab_cookie(onionCabHash);
          saveStorage();
          continue;
        }else
        {
          MERR("Onion cab: unable to find tk3 in jQuery");
          throw std::runtime_error("no_tk3");
        }
      }
      else{
        MERR("Onion cab: unable to find t5 in agreement script!");
        throw std::runtime_error("no_t5");
      }
    }
    return s;
  }
}

//Might throw exceptions!
std::string ManagerModule::fetchStaticUrl(const std::string& url)
{
  http::client::request request(url);
  request << header("Connection", "close");
  http::client::response response =
    client.get(request);
  return body(response);
}

std::string ManagerModule::fetchUrl(const std::string& url)
{
  if(url.find("onion.cab") != std::string::npos)
    return fetchOnionCabUrl(url);
  return fetchStaticUrl(url);
}

void ManagerModule::saveStorage()
{
  std::string data;
  if(!stor.SerializeToString(&data))
  {
    MERR("Unable to serialize storage to string!");
    return;
  }
  mInter->saveStorage(data);
}

//Base64
//^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$

int ManagerModule::fetchStaticModTable()
{
  MLOG("Fetching initial module tables from all sources...");
  for(auto str : sInfo.init_url())
  {
    try {
      MLOG("Trying to fetch table from "<<str<<"...");
      const std::string s = fetchUrl(str);
      MLOG("Body: "<<s);

      boost::match_results<std::string::const_iterator> results;
      boost::regex re("(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?");
      if (boost::regex_search(s.begin(), s.end(), results, re))
      {
        MLOG("Found a base64 encoded string: "<<std::string(results[1].first, results[1].second));
      }
      else
      {
        MLOG("No base64 strings...");
      }
    }catch(...)
    {
      MERR("Error occured while fetching from "<<str);
    }
  }
  MLOG("Done");
  return -1;
}

int ManagerModule::parseModuleInfo()
{
  std::string info = mInter->getModuleInfo();
  return sInfo.ParseFromString(info) == 0;
}

void ManagerModule::loadStorage()
{
  std::string* data = mInter->getStorage();
  if(data != NULL) stor.ParseFromString(*data);
}

bool running = true;
void ManagerModule::module_main()
{
  //Require persist module
  mInter->requireDependency(2526948902);
  mInter->commitDepsChanges();
  loadStorage();
  //mInter->relocateEverything("/tmp/testdir/");
  if(parseModuleInfo() != 0)
  {
    MERR("Unable to load module info...");
  }
  if(fetchStaticModTable() != 0)
  {
    MERR("Unable to fetch static module table from the internet...");
  }
  while(running)
  {
    try{
      //An interruption point
      boost::this_thread::sleep( boost::posix_time::milliseconds(2000));
    }
    catch(...)
    {
      MLOG("Interrupted, exiting...");
      break;
    }
  }
}

void* ManagerModule::getPublicInterface()
{
  return (void*)pInter;
}

ManagerInter::ManagerInter(ManagerModule* mod)
{
  this->mod = mod;
}

CHARLIE_CONSTRUCT(ManagerModule);
