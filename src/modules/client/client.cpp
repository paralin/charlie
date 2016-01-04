#include <Module.h>
#include <modules/client/Client.h>
#include <networking/EMsgSizes.h>
#include <networking/CharlieSession.h>
#include <charlie/random.h>
#include <charlie/SystemInfo.h>
#include <charlie/StateChangeEvent.h>
#include <charlie/machine_id.h>
#include <fstream>
#include <chrono>
#define MODULE_ID 2777954855

typedef std::chrono::high_resolution_clock Clock;

// 30 second allowed skew
#define ALLOWED_TIME_SKEW 30

#define DCASSERT(statement) if (!(statement)) { unexpectedDataReceived(); return; }
#define DCASSERTL(statement, msg) if (!(statement)) { MERR(msg); unexpectedDataReceived(); return; }
#define DCASSERTC(statement) if (!(statement)) { unexpectedDataReceived(); continue; }
#define DCASSERTLC(statement, msg) if (!(statement)) { MERR(msg); unexpectedDataReceived(); continue; }

using namespace modules::client;

ClientModule::ClientModule() :
  netModuleId(0),
  running(true),
  isDirect(false),
  pendingSocket(NULL),
  torModule(NULL),
  io_service(),
  resolver(io_service),
  mInter(NULL),
  pInter(new ClientInter(this)),
  nextConnectAttempt(0)
{
  MLOG("Client module constructed...");
}

ClientModule::~ClientModule()
{
}

void ClientModule::unexpectedDataReceived()
{
  MLOG("Unexpected data received, disconnecting.");
  session.reset();
}

// Select a sub-networking module
void ClientModule::selectNetworkModule()
{
  if (mInter == NULL)
    return;

  // Require all modules optionally
  std::vector<std::shared_ptr<charlie::CModule>> netModules = mInter->listModulesWithCap(charlie::MODULE_CAP_NET, true);
  netModuleIds.clear();
  if (netModules.empty())
  {
    MLOG("No known network proxy modules at this time.");
    return;
  }
  for (auto m : netModules)
  {
    MLOG("Possible proxy module: " << m->id());
    netModuleIds.insert(m->id());
    if (loadedModules.count(m->id()))
      loadedNetModules[m->id()] = (ModuleNet*) loadedModules[m->id()];
  }

  // Dont mess with a working connection
  if (session)
    return;

  // See if there's a better module to require explicitly
  std::shared_ptr<charlie::CModule> tmod = mInter->selectModule(netModules, NULL);
  if (tmod == NULL)
  {
    MLOG("Currently there are no suitable proxy modules.");
    return;
  }

  if (tmod->id() == netModuleId)
    return;

  // Select this module
  if (netModuleId != 0)
  {
    MLOG("Releasing explicit require for module " << netModuleId);
    mInter->releaseDependency(netModuleId);
    mInter->requireDependency(netModuleId, true);
  }

  netModuleId = tmod->id();
  MLOG("Selecting network proxy module " << netModuleId);
  mInter->requireDependency(netModuleId, false);
}

void ClientModule::shutdown()
{
  MLOG("Shutting down client module..");
  running = false;
}

void ClientModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;
}

void ClientModule::injectDependency(u32 id, void* dep)
{
  if(!dep || !id) return;
  MLOG("Dep injected "<<id);
  if (netModuleIds.count(id))
  {
    MLOG("Received network module " << id);
    loadedNetModules[id] = (ModuleNet*) dep;
  }
  loadedModules[id] = (ModuleAPI*) dep;
  MLOG("Stored loaded module " << id);
}

void ClientModule::releaseDependency(u32 id)
{
  MLOG("Dep released "<<id);
  if (loadedNetModules.count(id))
  {
    MLOG("Released network module " << id);
    loadedNetModules.erase(id);
  }
  if (netModuleId == id && usingNetModule)
  {
    MLOG("Network module we were using was dropped, disconnecting...");
    session.reset();
  }
  loadedModules.erase(id);
}

void* ClientModule::getPublicInterface()
{
  return pInter.get();
}

int ClientModule::parseModuleInfo()
{
  std::string info = mInter->getModuleInfo();
  return sInfo.ParseFromString(info) == 0;
}

void ClientModule::sendSystemInfo()
{
  SystemInfo* info = mInter->getSysInfo();

  CClientSystemInfo cinfo;
  cinfo.set_system_id(info->system_id);
  cinfo.set_hostname(getMachineName());
  cinfo.set_cpu_hash(info->cpu_hash);

  std::string data = cinfo.SerializeAsString();
  send(MODULE_ID, EClientEMsg_SystemInfo, 0, data);
}

// Main function
void ClientModule::module_main()
{
  parseModuleInfo();
  populateServerKeys();
  dependAllModules();

  selectNetworkModule();
  systemCrypto = mInter->getCrypto();

  MLOG("Parsing module info...");
  parseModuleInfo();

  while (running)
  {
    if (session)
    {
      if (isDirect)
      {
        auto t1 = Clock::now();
        io_service.poll_one();
        auto t2 = Clock::now();
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
        if (ms < 50)
          boost::this_thread::sleep(boost::posix_time::milliseconds(50 - ms));
        continue;
      }
      boost::this_thread::sleep(boost::posix_time::milliseconds(100));
    } else
    {
      if (tryConnectNetModules() || tryConnectAllEndpoints())
      {
        MLOG("Session acquired.");
      } else
      {
        boost::this_thread::sleep(boost::posix_time::milliseconds(3000));
      }
    }
  }
}

void ClientModule::handleMessage(charlie::CMessageHeader& head, std::string& data)
{
  if (head.emsg() == charlie::EMsgRoutedMessage)
  {
    DCASSERTL(head.has_target() && head.target().target_module() != 0, "Target module not specified, assuming something's wrong.");

    u32 tmod = head.target().target_module();

    // Find target module amongst loaded modules
    if (!loadedModules.count(tmod))
    {
      MERR("Message to module " << tmod << " not routeable.");
      charlie::CNetFailure fail;
      fail.set_fail_type(charlie::FAILURE_MODULE_NOTFOUND);
      std::string data = fail.SerializeAsString();
      send(charlie::EMsgFailure, data);
      return;
    }

    try
    {
      loadedModules[tmod]->handleCommand(head.target(), data);
    }
    catch (std::exception& ex)
    {
      MERR("Error when handleCommand() on " << tmod << ", " << ex.what());
      charlie::CNetFailure fail;
      fail.set_fail_type(charlie::FAILURE_EXCEPTION_RAISED);
      fail.set_error_message(ex.what());
      std::string data = fail.SerializeAsString();
      send(charlie::EMsgFailure, data);
      return;
    }
  }
  else if (head.emsg() == charlie::EMsgKeepalive)
  {
    charlie::CKeepAlive ka;
    std::string d = ka.SerializeAsString();
    send(charlie::EMsgKeepalive, d);
  }
  else
  {
    MERR("Don't know how to handle emsg " << head.emsg());
  }
}

void ClientModule::send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target)
{
  session->send(emsg, data, target);
}

void ClientModule::send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data)
{
  session->send(targetModule, targetEmsg, jobid, data);
}

bool ClientModule::tryConnectNetModules()
{
  MLOG("Currently " << loadedNetModules.size() << " loaded net modules.");
  for (auto p : loadedNetModules)
  {
    auto m = p.second;
    if (m->ready())
    {
      MLOG("Module " << p.first << " ready.");
      pendingSocket.reset();
      session = m->getSession();
      session->setController(this);
      isDirect = false;
      torModule = m;

      if (!session)
      {
        MLOG("... but network module returned null pendingSocket.");
        m->disconnectInvalid();
        continue;
      } else
      {
        MLOG("Received session from tor.");
      }

      return true;
    } else
    {
      MLOG("Network module " << p.first << " is not ready.");
    }
  }
  return false;
}

bool ClientModule::tryConnectAllEndpoints()
{
  for (int i = 0; i < sInfo.server_addr_size(); i++)
  {
    std::string endp = sInfo.server_addr(i);
    MLOG("Attempting to connect to " << endp << "...");
    if (tryConnectEndpoint(endp.c_str()))
    {
      MLOG("Connection successful...");
      isDirect = true;
      unsigned char* key;
      int klen = mInter->getCrypto()->getLocalPriKey(&key);
      std::string ke((char*) key, klen);
      free(key);
      session = std::make_shared<CharlieSession>(pendingSocket, this, ke);
      session->start();
      pendingSocket.reset();
      return true;
    }
  }
  return false;
}

void ClientModule::onDisconnected()
{
  MLOG("Disconnected.");
  session.reset();
}

void ClientModule::onHandshakeComplete()
{
}

bool ClientModule::tryConnectEndpoint(const char* endp)
{
  // Parse out the ip address and port
  std::string port("9922");
  std::string ipadd(endp);
  {
    int colidx = ipadd.find(":");
    if (colidx != std::string::npos)
    {
      port = ipadd.substr(colidx + 1);
      ipadd = ipadd.substr(0, colidx);
    }
  }

  MLOG("addr: " << ipadd << " port: " << port);

  try {
    tcp::resolver::query query(ipadd, port, boost::asio::ip::resolver_query_base::numeric_service);
    tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);

    pendingSocket = std::make_shared<tcp::socket>(io_service);
    try {
      boost::asio::connect(*pendingSocket, endpoint_iterator);
      return true;
    }
    catch (const boost::system::error_code& ex)
    {
      MERR("Unable to connect, error: " << ex);
      if (pendingSocket)
        pendingSocket.reset();
      return false;
    }
  }
  catch (std::exception& ex)
  {
    MERR("Overall error trying to connect "<< ex.what());
    return false;
  }
}

void ClientModule::populateServerKeys()
{
  for (int i = 0; i < sInfo.server_key_size(); i++)
    serverKeys.insert(sInfo.server_key(i));
}

void ClientModule::handleEvent(u32 eve, void* data)
{
  charlie::EModuleEvents event = (charlie::EModuleEvents) eve;
  if (event == charlie::EVENT_MODULE_TABLE_RELOADED)
  {
    dependAllModules();
    selectNetworkModule();
  }
}

// Depend on everything optionally
// .. so we can route messages
void ClientModule::dependAllModules()
{
  charlie::CModuleTable* tab = mInter->getModuleTable();
  for (int i = 0; i < tab->signed_modules_size(); i++)
  {
    charlie::CModule m;
    if (!m.ParseFromString(tab->signed_modules(i).data()))
      continue;
    mInter->requireDependency(m.id(), true);
  }
}

void ClientModule::retryConnectionsNow()
{
  // yep
  nextConnectAttempt = 0;
}

ClientInter::ClientInter(ClientModule * mod)
{
  this->mod = mod;
}

ClientInter::~ClientInter()
{
}

void ClientInter::send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target)
{
  mod->send(emsg, data, target);
}

void ClientInter::send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data)
{
  mod->send(targetModule, targetEmsg, jobid, data);
}

void ClientInter::retryConnectionsNow()
{
  mod->retryConnectionsNow();
}

void ClientInter::handleCommand(const charlie::CMessageTarget& targ, std::string& command)
{
  if (targ.emsg() == EClientEMsg_RequestSystemInfo)
  {
    MLOG("Received request for system info.");
    mod->sendSystemInfo();
    return;
  }
  MERR("Received command " << targ.emsg() << " for this module but no commands are defined.");
}

CHARLIE_CONSTRUCT(ClientModule);
