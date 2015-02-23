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
    if(!onionCabHash.empty())
    {
      std::string cookie = (std::string("onion_cab_iKnowShit=")+onionCabHash);
      request << header("Cookie", cookie);
    }
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

//Base64
//^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$

int ManagerModule::fetchStaticModTable()
{
  MLOG("Fetching initial module tables from all sources...");
  for(auto str : sInfo.init_url())
  {
    try {
      MLOG("Trying to fetch table from "<<str<<"...");
      std::string s = fetchUrl(str);

      boost::regex re("^(?:[A-Za-z0-9+/]{4})*(?:[A-Za-z0-9+/]{2}==|[A-Za-z0-9+/]{3}=)?$", boost::regex_constants::basic);
      auto begin =
        boost::sregex_iterator(s.begin(), s.end(), re);
      auto end = boost::sregex_iterator();

      MLOG("Found " << std::distance(begin, end)<<" words:");

      for (boost::sregex_iterator i = begin; i != end; ++i) {
        boost::smatch match = *i;
        std::string match_str = match.str();
        MLOG(match_str);
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

bool running = true;
void ManagerModule::module_main()
{
  //Require persist module
  mInter->requireDependency(2526948902);
  mInter->commitDepsChanges();
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
