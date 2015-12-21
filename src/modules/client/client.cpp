#include <Module.h>
#include <modules/client/Client.h>
#include <networking/EMsgSizes.h>
#include <charlie/random.h>
#include <charlie/SystemInfo.h>
#include <charlie/StateChangeEvent.h>

// 30 second allowed skew
#define ALLOWED_TIME_SKEW 30

// Depend on all modules optionally.
#define DEPEND_ALL_MODULES

#define DCASSERT(statement) if (!(statement)) { unexpectedDataReceived(); return; }
#define DCASSERTL(statement, msg) if (!(statement)) { MERR(msg); unexpectedDataReceived(); return; }
#define DCASSERTC(statement) if (!(statement)) { unexpectedDataReceived(); continue; }
#define DCASSERTLC(statement, msg) if (!(statement)) { MERR(msg); unexpectedDataReceived(); continue; }

using namespace modules::client;

ClientModule::ClientModule() :
  netModuleId(0),
  running(true),
  socket(NULL),
  resolver(NULL),
  io_service(NULL),
  sessionCrypto(NULL),
  mInter(NULL),
  pInter(NULL)
{
  MLOG("Client module constructed...");
  pInter = new ClientInter(this);
  clientChallenge = gen_random(10);
  // This will set everything to an init value
  releaseNetworking();
}

ClientModule::~ClientModule()
{
  delete pInter;
  releaseNetworking();
}

void ClientModule::releaseNetworking()
{
  disconnect();
  socket = NULL;
  if (io_service)
  {
    delete io_service;
  }
  if (resolver)
  {
    delete resolver;
  }
  io_service = NULL;
  resolver = NULL;
}

void ClientModule::disconnect()
{
  if (socket)
  {
    delete socket;
    socket = NULL;
  }
  connected = false;
  wasConnected = false;
  handshakeComplete = false;
  expectingHeaderLengthPrefix = true;
  expectingHeader = false;
  receivedServerIdentify = false;
  sentClientIdentify = false;
  usingNetModule = false;
  head.Clear();
  body.Clear();
  serverChallenge = "";
  resetReceiveContext();
  if (sessionCrypto)
  {
    delete sessionCrypto;
    sessionCrypto = NULL;
  }
}

void ClientModule::resetReceiveContext()
{
  expectingHeaderLengthPrefix = true;
  expectingHeader = false;
  expectedBodySize = 0;
}

// Select a sub-networking module
void ClientModule::selectNetworkModule()
{
  if (mInter == NULL)
    return;

  // Require all modules optionally
  auto netModules = mInter->listModulesWithCap(charlie::MODULE_CAP_NET, true);
#ifndef DEPEND_ALL_MODULES
  std::set<u32> oldNetModuleIds(netModuleIds);
#endif
  netModuleIds.clear();
  if (netModules.empty())
  {
    MLOG("No known network proxy modules at this time.");
    return;
  }
  for (auto m : netModules)
  {
    MLOG("Possible proxy module: " << m);
    netModuleIds.insert(m->id());
#ifndef DEPEND_ALL_MODULES
    // this will immediately insert the dep
    mInter->requireDependency(m->id(), false);
#endif
  }

#ifndef DEPEND_ALL_MODULES
  for (auto m : oldNetModuleIds)
  {
    if (!netModuleIds.count(m))
      mInter->releaseDependency(m);
  }
#endif

  // Dont mess with a working connection
  if (connected)
    return;

  // See if there's a better module to require explicitly
  charlie::CModule* tmod = mInter->selectModule(netModules, NULL);
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
    mInter->requireDependency(netModuleId, false);
  }

  netModuleId = tmod->id();
  MLOG("Selecting network proxy module " << netModuleId);
  mInter->releaseDependency(netModuleId);
  mInter->requireDependency(netModuleId, true);
}

void ClientModule::initNetworking()
{
  MLOG("Init io_service...");
  io_service = new boost::asio::io_service();
  MLOG("Init resolver...");
  resolver = new tcp::resolver(*io_service);
  MLOG("Ready to start connection attempts.");
}

void ClientModule::shutdown()
{
  MLOG("Shutting down directconnect module..");
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
  if (id == 3133916783)
  {
    manager = (modules::manager::ManagerInter*) dep;
    managerMtx.unlock();
  }
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
  if (id == 3133916783)
  {
    manager = NULL;
    managerMtx.lock();
  }
  if (loadedNetModules.count(id))
  {
    MLOG("Released network module " << id);
    loadedNetModules.erase(id);
  }
  if (netModuleId == id && usingNetModule)
  {
    MLOG("Network module we were using was dropped, disconnecting...");
    disconnect();
  }
  loadedModules.erase(id);
}

void* ClientModule::getPublicInterface()
{
  return pInter;
}

int ClientModule::parseModuleInfo()
{
  std::string info = mInter->getModuleInfo();
  return sInfo.ParseFromString(info) == 0;
}

void ClientModule::unexpectedDataReceived()
{
  MLOG("Unexpected data received, disconnect.");
  // disconnect();
  resetReceiveContext();
  connected = false;
  wasConnected = true;
}

// Main function
void ClientModule::module_main()
{
#ifdef DEPEND_ALL_MODULES
  {
    auto allMods = mInter->listModuleInstances();
    for (auto mod : allMods)
    {
      if (mod.status() == charlie::MODULE_LOADED || mod.status() == charlie::MODULE_LOADED_RUNNING)
      {
        MLOG("Pulling in loaded module " << mod.id());
        mInter->requireDependency(mod.id(), true);
      }
    }
  }
#endif

  selectNetworkModule();
  systemCrypto = mInter->getCrypto();

  //MLOG("Waiting for manager module...");
  //populateServerKeys();
  MLOG("Parsing module info...");
  parseModuleInfo();
  MLOG("Initializing networking...");
  initNetworking();

  // Main state loop
  try {
    while (running)
    {
      if (!connected)
      {
        if (wasConnected)
        {
          disconnect();
          resetReceiveContext();
          MLOG("Waiting 3 seconds to re-try...");
          boost::this_thread::sleep(boost::posix_time::seconds(3));
        }
        if (tryConnectNetModules() || tryConnectAllEndpoints())
        {
          connected = true;
          wasConnected = true;
          resetReceiveContext();
        }
        else
        {
          MLOG("Connections unsuccessful, will try again...");
          boost::this_thread::sleep(boost::posix_time::seconds(10));
        }
        continue;
      }

      // expected size of incoming data
      u32 bufSize;
      // Header length prefix = 32 bit = 4 byte unsigned integer.
      if (expectingHeaderLengthPrefix)
        bufSize = 4;
      else if (expectingHeader)
        bufSize = expectedHeaderSize;
      else
        bufSize = expectedBodySize;

      buf.resize(bufSize);
      boost::system::error_code error;

      size_t len = socket->read_some(boost::asio::buffer(buf), error);
      if (error == boost::asio::error::eof)
      {
        MLOG("Clean disconnect by server.");
        disconnect();
        continue;
      }
      else if (error)
      {
        MERR("Unexpected disconnect with error " << error);
        disconnect();
        continue;
      }

      MLOG("Received " << bufSize << " of data.");
      if (expectingHeaderLengthPrefix)
      {
        expectingHeaderLengthPrefix = false;
        expectingHeader = true;
        expectedHeaderSize = 0;
        for (unsigned i = 0; i < 4; ++i)
          expectedHeaderSize = expectedHeaderSize * 256 + (static_cast<char>(buf[i]) & 0xFF);
        DCASSERTLC(expectedHeaderSize < 80, "Expected header size = " << expectedHeaderSize << " which is too big.");
      } else if (expectingHeader)
      {
        head.Clear();

        DCASSERTLC(head.ParseFromArray(&buf[0], buf.size()), "CMessageHeader parse failed.");
        DCASSERTLC(validateMessageHeader(), "Message header failed validation.");

        expectingHeader = false;
        expectedBodySize = head.body_size();
      } else
      {
        body.Clear();
        DCASSERTLC(body.ParseFromArray(&buf[0], buf.size()), "CMessageBody parse failed.");

        // Weve already validated the emsg before
        if (!handshakeComplete && head.emsg() == charlie::EMsgServerIdentify)
        {
          DCASSERTLC(handleServerIdentify(), "Invalid server identify.");
          receivedServerIdentify = true;
        }

        DCASSERTLC(validateMessageBody(), "Message body validation failed.");

        // Everything after this should be encrypted
        if (receivedServerIdentify && sentClientIdentify)
        {
          unsigned char* decrypted;

          int len = decryptRsaBuf((charlie::CRSABuffer*)&body.rsa_body(), systemCrypto, &decrypted, false);
          DCASSERTLC(len != FAILURE, "Unable to decrypt rsa buffer.");

          std::string data((char*) decrypted, len);
          free(decrypted);
          handleMessage(data);
        }
        else
          sendClientIdentify();

        resetReceiveContext();
      }
    }
  } catch (std::exception& ex)
  {
    MLOG("Caught overall exception " << ex.what());
  }

  MLOG("Releasing networking...");
  releaseNetworking();
}

void ClientModule::sendClientIdentify()
{
  charlie::CClientIdentify ident;
  unsigned char* sig;
  unsigned char* pubkey;

  int sigLen = systemCrypto->digestSign((const unsigned char*) serverChallenge.c_str(), serverChallenge.length(), &sig, false);
  int len = systemCrypto->getLocalPubKey(&pubkey);

  ident.set_client_pubkey(pubkey, len);
  ident.set_client_challenge(clientChallenge);
  ident.set_challenge_response(sig, sigLen);

  free(pubkey);
  free(sig);

  std::string outp = ident.SerializeAsString();
  sentClientIdentify = true;
  send(charlie::EMsgClientIdentify, 0, outp);
}

void ClientModule::handleMessage(std::string& data)
{
  if (!handshakeComplete)
  {
    if (head.emsg() == charlie::EMsgServerAccept)
      handleServerAccept(data);
    else
      DCASSERTL(false, "Unknown emsg during handshake: " << head.emsg());
    return;
  }
  else if (head.emsg() == charlie::EMsgRoutedMessage)
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
      send(charlie::EMsgFailure, head.target().target_module(), data, head.target().job_id());
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
      send(charlie::EMsgFailure, head.target().target_module(), data, head.target().job_id());
      return;
    }
  }
  else
  {
    MERR("Don't know how to handle emsg " << head.emsg());
  }
}

void ClientModule::handleServerAccept(std::string& data)
{
  charlie::CServerAccept ident;

  // Parse
  DCASSERTL(ident.ParseFromString(data), "Unable to parse CServerAccept.");

  // Validate the server challenge response
  DCASSERTL(ident.has_challenge_response(), "Server didn't send challenge response.");
  DCASSERTL(sessionCrypto->digestVerify((const unsigned char*) clientChallenge.c_str(), clientChallenge.length(), (unsigned char*) ident.challenge_response().c_str(), true), "Invalid server challenge response.");

  sendClientAccept(ident.request_system_info());
  handshakeComplete = true;
  MLOG("Handshake complete.");
}

void ClientModule::sendClientAccept(bool sendInfo)
{
  MLOG("Accepting the server.");
  charlie::CClientAccept accp;
  if (sendInfo)
  {
    SystemInfo* sysInfo = mInter->getSysInfo();
    charlie::CNetSystemInfo* info = accp.mutable_system_info();
    info->set_cpu_hash(sysInfo->cpu_hash);
    info->set_system_id(sysInfo->system_id);
  }

  std::string outp = accp.SerializeAsString();
  send(charlie::EMsgClientAccept, 0, outp);
}

void ClientModule::send(u32 targetModule, u32 jobId, u32 targetEmsg, std::string& data)
{
  send(charlie::EMsgRoutedMessage, targetModule, data, jobId, targetEmsg); 
}

void ClientModule::send(charlie::EMsg emsg, u32 targetModule, std::string& data, u32 jobid, u32 targetEmsg)
{
  charlie::CMessageBody nBody;

  // Timestamp code
  {
    std::time_t now;
    std::time(&now);
    u32 timestamp = now - timeConnected;
    nBody.set_timestamp(timestamp);
    unsigned char* tsBuf = (unsigned char*) malloc(sizeof(unsigned char) * 4);
    tsBuf[0] = static_cast<unsigned char>((timestamp >> 24) & 0xFF);
    tsBuf[1] = static_cast<unsigned char>((timestamp >> 16) & 0xFF);
    tsBuf[2] = static_cast<unsigned char>((timestamp >> 8) & 0xFF);
    tsBuf[3] = static_cast<unsigned char>(timestamp & 0xFF);

    unsigned char* tsSigBuf;
    int digestLen = systemCrypto->digestSign((const unsigned char*) tsBuf, 4, &tsSigBuf, false);
    free(tsBuf);
    nBody.set_timestamp_signature(tsSigBuf, digestLen);
    free(tsSigBuf);
  }

  charlie::CRSABuffer* buf = new charlie::CRSABuffer();
  encryptRsaBuf(buf, sessionCrypto, (const unsigned char*) data.c_str(), data.length(), true);
  nBody.set_allocated_rsa_body(buf);

  {
    const std::string& toSign = nBody.rsa_body().data();
    unsigned char* bodySigBuf;
    int digestLen = systemCrypto->digestSign((const unsigned char*) toSign.c_str(), toSign.length(), &bodySigBuf, false);
    nBody.set_signature((const char*) bodySigBuf, digestLen);
    free(bodySigBuf);
  }

  std::string sBody = nBody.SerializeAsString();

  charlie::CMessageHeader nHead;
  nHead.set_emsg(emsg);
  nHead.set_body_size(sBody.length());
  if (targetModule)
  {
    charlie::CMessageTarget* target = new charlie::CMessageTarget();
    target->set_target_module(targetModule);
    target->set_emsg(targetEmsg);
    nHead.set_allocated_target(target);
  }

  std::string sHead = nHead.SerializeAsString();

  unsigned char* hlenBuf = (unsigned char*) malloc(sizeof(unsigned char) * 4);
  hlenBuf[0] = static_cast<unsigned char>((sHead.length() >> 24) & 0xFF);
  hlenBuf[1] = static_cast<unsigned char>((sHead.length() >> 16) & 0xFF);
  hlenBuf[2] = static_cast<unsigned char>((sHead.length() >> 8) & 0xFF);
  hlenBuf[3] = static_cast<unsigned char>(sHead.length() & 0xFF);
  std::string hlenBuff ((char*) hlenBuf, 4);
  free(hlenBuf);

  boost::asio::write(*socket, boost::asio::buffer(hlenBuff));
  boost::asio::write(*socket, boost::asio::buffer(sHead));
  boost::asio::write(*socket, boost::asio::buffer(sBody));
}

bool ClientModule::validateMessageHeader()
{
  if (!charlie::EMsg_IsValid(head.emsg()))
  {
    MERR("EMsg type " << head.emsg() << " is invalid.");
    return false;
  }

  // Validate handshake phase
  if (!handshakeComplete)
  {
    charlie::EMsg expected;
    // Expecting a CServerIdentify if !handshakeComplete and !receivedServerIdentify
    if (!receivedServerIdentify)
      expected = charlie::EMsgServerIdentify;
    else
      expected = charlie::EMsgServerAccept;

    if (expected != head.emsg())
    {
      MERR("Unexpected message " << head.emsg() << " during handshake phase.");
      return false;
    }
  }

  // Verify it is less than the known maximum
  {
    std::map<charlie::EMsg, int>::iterator it = charlie::networking::StaticDefinitions::EMsgSizes.find(head.emsg());
    // 50kb max size default
    int maxSize = 50000;
    if (it != charlie::networking::StaticDefinitions::EMsgSizes.end())
      maxSize = it->second;
    if (head.body_size() > maxSize)
    {
      MERR("Size for " << head.emsg() << " - " << head.body_size() << " bytes > expected maximum of " << maxSize << " bytes.");
      return false;
    }
  }

  // Validate the target module
  // XXX
  return true;
}

bool ClientModule::validateMessageBody()
{
  if (!body.has_signature())
  {
    MERR("Body is missing contents signature.");
    return false;
  }

  if (!body.has_timestamp_signature())
  {
    MERR("Body is missing timestamp signature.");
    return false;
  }

  if (!body.has_timestamp())
  {
    MERR("Body is missing timestamp.");
    return false;
  }

  {
    std::time_t now;
    std::time(&now);
    now -= timeConnected;
    std::time_t timestamp = body.timestamp();
    std::time_t diff = std::abs(now - timestamp);
    if (diff > ALLOWED_TIME_SKEW)
    {
      MERR("Time skew of " << diff << " greater than " << ALLOWED_TIME_SKEW << ".");
      return false;
    }

    unsigned char* tsBuf = (unsigned char*) malloc(sizeof(unsigned char) * 4);
    tsBuf[0] = static_cast<unsigned char>((timestamp >> 24) & 0xFF);
    tsBuf[1] = static_cast<unsigned char>((timestamp >> 16) & 0xFF);
    tsBuf[2] = static_cast<unsigned char>((timestamp >> 8) & 0xFF);
    tsBuf[3] = static_cast<unsigned char>(timestamp & 0xFF);
    if (sessionCrypto->digestVerify((unsigned char*) tsBuf, 4, (unsigned char*) body.timestamp_signature().c_str(), body.timestamp_signature().length(), true) != SUCCESS)
    {
      MERR("Digest verification of timestamp incorrect.");
      return false;
    }
    MLOG("Verification of signature for timestamp " << timestamp << " vs. " << now << " succeeded.");
  }

  bool rsaBody = false;
  if (!body.has_rsa_body())
  {
    if (body.has_unenc_body())
    {
      if (head.emsg() != charlie::EMsgServerIdentify)
      {
        MERR("Only EMsgServerIdentify should be unencrypted.");
        return false;
      }
      rsaBody = false;
    } else {
      MERR("Body has no contents!");
      return false;
    }
  } else
    rsaBody = true;

  const std::string& contents = rsaBody ? body.rsa_body().data() : body.unenc_body();

  // Verify the signature
  if (sessionCrypto->digestVerify((const unsigned char*) contents.c_str(), contents.length(), (unsigned char*) body.signature().c_str(), body.signature().length(), true) != SUCCESS)
  {
    MERR("Incorrect signature for contents.");
    return false;
  }

  return true;
}

bool ClientModule::handleServerIdentify()
{
  // We expect unencrypted data
  if (!body.has_unenc_body())
  {
    MERR("Received server identify without unenc body, invalid.");
    return false;
  }

  // We expect a signature
  if (!body.has_unenc_body())
  {
    MERR("Received server identify without unenc body, invalid.");
    return false;
  }

  // Load up the message
  charlie::CServerIdentify ident;
  if (!ident.ParseFromString(body.unenc_body()))
  {
    MERR("Unable to parse CServerIdentify from body.");
    return false;
  }

  if (!ident.has_server_pubkey() || !ident.has_server_challenge())
  {
    MERR("Server identify is missing challenge or pubkey.");
    return false;
  }

  if (sessionCrypto)
    delete sessionCrypto;

  sessionCrypto = new Crypto();
  MLOG("Setting remote pubkey...");
  if (sessionCrypto->setRemotePubKey((unsigned char*)ident.server_pubkey().c_str(), ident.server_pubkey().length()) != SUCCESS)
  {
    MERR("Unable to parse remote public key");
    return false;
  }
  MLOG("Remote pubkey set...");
  // Todo: validate this pubkey against stored pubkey

  // Validate against this pubkey later
  serverChallenge = ident.server_challenge();
  return true;
}

bool ClientModule::tryConnectNetModules()
{
  for (auto p : loadedNetModules)
  {
    auto m = p.second;
    if (m->ready())
    {
      MLOG("Module " << p.first << " ready.");
      socket = m->getSocket();
      if (socket == NULL)
      {
        MLOG("... but network module returned null socket.");
        m->disconnectInvalid();
        continue;
      }
      return true;
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
      std::time(&timeConnected);
      return true;
    }
  }
  return false;
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
    tcp::resolver::iterator endpoint_iterator = resolver->resolve(query);

    socket = new boost::asio::ip::tcp::socket(*io_service);
    try {
      boost::asio::connect(*socket, endpoint_iterator);
      return true;
    }
    catch (const boost::system::error_code& ex)
    {
      MERR("Unable to connect, error: " << ex);
      delete socket;
      socket = NULL;
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
  if (manager == NULL) return;
  modules::manager::CManagerInfo* info = manager->getInfo();
  if (info == NULL) return;
  for (int i = 0; i < info->server_key_size(); i++)
    serverKeys.insert(info->server_key(i));
}

void ClientModule::handleEvent(u32 eve, void* data)
{
  charlie::EModuleEvents event = (charlie::EModuleEvents) eve;
  if (event == charlie::EVENT_MODULE_TABLE_RELOADED)
      selectNetworkModule();
  else if (event == charlie::EVENT_MODULE_STATE_CHANGE)
  {
    ModuleStateChange* ch = (ModuleStateChange*) data;
#ifndef DEPEND_ALL_MODULES
    if (netModuleIds.count(ch->mod))
      return;
#endif
    if (ch->state == charlie::MODULE_LOADED || ch->state == charlie::MODULE_LOADED_RUNNING)
      mInter->requireDependency(ch->mod, true);
    else
      mInter->releaseDependency(ch->mod);
  }
}

ClientInter::ClientInter(ClientModule * mod)
{
  this->mod = mod;
}

ClientInter::~ClientInter()
{
}

void ClientInter::handleCommand(const charlie::CMessageTarget& targ, std::string& command)
{
  MERR("Received command " << targ.emsg() << " for this module but no commands are defined.");
}

CHARLIE_CONSTRUCT(ClientModule);
