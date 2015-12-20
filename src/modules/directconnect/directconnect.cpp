#include <Module.h>
#include <modules/directconnect/DirectConnect.h>
#include <networking/EMsgSizes.h>
#include <charlie/random.h>
#include <charlie/SystemInfo.h>

// 30 second allowed skew
#define ALLOWED_TIME_SKEW 30

#define DCASSERT(statement) if (!(statement)) { unexpectedDataReceived(); return; }
#define DCASSERTL(statement, msg) if (!(statement)) { MERR(msg); unexpectedDataReceived(); return; }
#define DCASSERTC(statement) if (!(statement)) { unexpectedDataReceived(); continue; }
#define DCASSERTLC(statement, msg) if (!(statement)) { MERR(msg); unexpectedDataReceived(); continue; }

using namespace modules::directconnect;

DirectConnectModule::DirectConnectModule()
{
  MLOG("Direct connect socket net module constructed...");
  pInter = new DirectConnectInter(this);
  running = true;
  socket = NULL;
  resolver = NULL;
  io_service = NULL;
  sessionCrypto = NULL;
  mInter = NULL;
  pInter = NULL;
  clientChallenge = gen_random(10);
  // This will set everything to an init value
  releaseNetworking();
}

DirectConnectModule::~DirectConnectModule()
{
  delete pInter;
  releaseNetworking();
}

void DirectConnectModule::releaseNetworking()
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

void DirectConnectModule::disconnect()
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

void DirectConnectModule::resetReceiveContext()
{
  expectingHeaderLengthPrefix = true;
  expectingHeader = false;
  expectedBodySize = 0;
}

void DirectConnectModule::initNetworking()
{
  MLOG("Init io_service...");
  io_service = new boost::asio::io_service();
  MLOG("Init resolver...");
  resolver = new tcp::resolver(*io_service);
  MLOG("Ready to start connection attempts.");
}

void DirectConnectModule::shutdown()
{
  MLOG("Shutting down directconnect module..");
  running = false;
}

void DirectConnectModule::setModuleInterface(ModuleInterface* inter)
{
  MLOG("Received module interface");
  mInter = inter;
}

void DirectConnectModule::injectDependency(u32 id, void* dep)
{
  if(!dep || !id) return;
  MLOG("Dep injected "<<id);
  if (id == 3133916783)
  {
    manager = (modules::manager::ManagerInter*) dep;
    managerMtx.unlock();
  }
}

void DirectConnectModule::releaseDependency(u32 id)
{
  MLOG("Dep released "<<id);
  if (id == 3133916783)
  {
    manager = NULL;
    managerMtx.lock();
  }
}

void* DirectConnectModule::getPublicInterface()
{
  return pInter;
}

int DirectConnectModule::parseModuleInfo()
{
  std::string info = mInter->getModuleInfo();
  return sInfo.ParseFromString(info) == 0;
}

void DirectConnectModule::unexpectedDataReceived()
{
  MLOG("Unexpected data received, disconnect.");
  // disconnect();
  resetReceiveContext();
  connected = false;
  wasConnected = true;
}

// Main function
void DirectConnectModule::module_main()
{
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
        if (tryConnectAllEndpoints())
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

void DirectConnectModule::sendClientIdentify()
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

void DirectConnectModule::handleMessage(std::string& data)
{
  if (!handshakeComplete)
  {
    if (head.emsg() == charlie::EMsgServerAccept)
      handleServerAccept(data);
  }
}

void DirectConnectModule::handleServerAccept(std::string& data)
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

void DirectConnectModule::sendClientAccept(bool sendInfo)
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

void DirectConnectModule::send(charlie::EMsg emsg, u32 targetModule, std::string& data)
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
  nHead.set_target_module(targetModule);
  nHead.set_body_size(sBody.length());

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

bool DirectConnectModule::validateMessageHeader()
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

bool DirectConnectModule::validateMessageBody()
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

bool DirectConnectModule::handleServerIdentify()
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

bool DirectConnectModule::tryConnectAllEndpoints()
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

bool DirectConnectModule::tryConnectEndpoint(const char* endp)
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

void DirectConnectModule::populateServerKeys()
{
  if (manager == NULL) return;
  modules::manager::CManagerInfo* info = manager->getInfo();
  if (info == NULL) return;
  for (int i = 0; i < info->server_key_size(); i++)
    serverKeys.insert(info->server_key(i));
}

void DirectConnectModule::handleEvent(u32 eve, void* data)
{
}

DirectConnectInter::DirectConnectInter(DirectConnectModule * mod)
{
  this->mod = mod;
}

bool DirectConnectInter::ready()
{
  return false;
}


DirectConnectInter::~DirectConnectInter()
{
}

void DirectConnectInter::handleCommand(void* command)
{
}

CHARLIE_CONSTRUCT(DirectConnectModule);
