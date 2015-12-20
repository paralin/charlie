#include <cserver/System.h>
#include <charlie/random.h>
#include <cserver/NetHost.h>
#include <cserver/networking/CharlieClient.h>
#include <charlie/Crypto.h>
#include <charlie/CryptoBuf.h>
#include <Logging.h>

#define ALLOWED_TIME_SKEW 30

CharlieClient::CharlieClient(std::shared_ptr<tcp::socket>& socket, NetHost* host)
  : socket(socket)
{
  this->host = host;
  expectingHeaderLengthPrefix = true;
  expectingHeader = false;
  handshakeComplete = false;
  receivedClientIdentify = false;
  sessionCrypto = NULL;
  serverChallenge = gen_random(10);
  serverCrypto = host->sys->crypt;
  std::time(&timeConnected);
}

void CharlieClient::start()
{
  CLOG("Sending server identify...");
  sendServerIdentify();
  doRead();
}

void CharlieClient::sendServerIdentify()
{
  charlie::CServerIdentify ident;
  ident.set_server_challenge(serverChallenge);
  unsigned char* buf;
  int len = serverCrypto->getLocalPubKey(&buf);
  ident.set_server_pubkey(buf, len);
  free(buf);
  std::string data = ident.SerializeAsString();
  send(charlie::EMsgServerIdentify, 0, data);
}

void CharlieClient::send(charlie::EMsg emsg, u32 targetModule, std::string& data)
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
    int digestLen = serverCrypto->digestSign((const unsigned char*) tsBuf, 4, &tsSigBuf, false);
    free(tsBuf);
    nBody.set_timestamp_signature(tsSigBuf, digestLen);
    free(tsSigBuf);
  }

  if (emsg == charlie::EMsgServerIdentify)
    nBody.set_unenc_body((const unsigned char*) data.c_str(), data.length());
  else
  {
    charlie::CRSABuffer* buf = new charlie::CRSABuffer();
    encryptRsaBuf(buf, sessionCrypto, (const unsigned char*) data.c_str(), data.length(), true);
    nBody.set_allocated_rsa_body(buf);
  }

  {
    const std::string& toSign = nBody.has_rsa_body() ? nBody.rsa_body().data() : nBody.unenc_body();
    unsigned char* bodySigBuf;
    int digestLen = serverCrypto->digestSign((const unsigned char*) toSign.c_str(), toSign.length(), &bodySigBuf, false);
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

void CharlieClient::doRead()
{
  auto self(shared_from_this());
  u32 bufSize;

  // Header prefix is 4 bytes
  if (expectingHeaderLengthPrefix)
    bufSize = 4;
  else if (expectingHeader)
    bufSize = expectedHeaderSize;
  else
    bufSize = expectedBodySize;

  buffer.resize(bufSize);
  boost::asio::async_read(*socket,
      boost::asio::buffer(buffer),
      [this, self](boost::system::error_code ec, std::size_t /*length*/)
      {
        if (!ec)
          handleReadData();
        else
        {
          CERR("Error code " << ec << " receiving.");
          disconnect();
        }
      });
}

void CharlieClient::handleReadData()
{
  auto self(shared_from_this());
  CLOG("Received " << buffer.size() << " bytes.");
  if (expectingHeaderLengthPrefix)
  {
    expectingHeaderLengthPrefix = false;
    expectingHeader = true;
    expectedHeaderSize = 0;
    for (unsigned i = 0; i < 4; ++i)
      expectedHeaderSize = expectedHeaderSize * 256 + (static_cast<char>(buffer[i]) & 0XFF);
    if (expectedHeaderSize > 80)
    {
      CERR("Received header size of " << expectedHeaderSize << ", too big.");
      disconnect();
      return;
    }
  }
  else if (expectingHeader)
  {
    head.Clear();
    if (!head.ParseFromArray(&buffer[0], buffer.size()))
    {
      CERR("CMessageHeader parse failed.");
      disconnect();
      return;
    }

    if (!validateMessageHeader())
    {
      CERR("Messgae header failed to validate.");
      disconnect();
      return;
    }

    expectingHeader = false;
    expectedBodySize = head.body_size();
  } else
  {
    body.Clear();
    if (!body.ParseFromArray(&buffer[0], buffer.size()))
    {
      CERR("CMessageBody parse failed.");
      disconnect();
      return;
    }

    unsigned char* decrypted = NULL;
    if (head.emsg() != charlie::EMsgClientIdentify && !validateMessageBody())
    {
      CERR("CMessageBody verification failed.");
      disconnect();
      return;
    }

    int decLen = decryptRsaBuf((charlie::CRSABuffer*) &body.rsa_body(), serverCrypto, &decrypted, false);
    if (decLen == FAILURE)
    {
      CERR("Unable to decrypt rsa buffer.");
      disconnect();
      return;
    }
    std::string data((char*) decrypted, decLen);
    free(decrypted);

    expectingHeaderLengthPrefix = true;
    expectingHeader = false;
    expectedBodySize = 0;
    expectedHeaderSize = 0;

    handleMessage(data);
  }

  doRead();
}

void CharlieClient::handleMessage(std::string& data)
{
  switch (head.emsg())
  {
    case charlie::EMsgClientIdentify:
      if (!handleClientIdentify(data))
      {
        CERR("Client identify invalid.");
        disconnect();
        return;
      }
      break;
    case charlie::EMsgClientAccept:
      handleClientAccept(data);
      break;
    default:
      CERR("Unrecognized emsg " << head.emsg());
      disconnect();
      return;
  }
}

bool CharlieClient::validateMessageHeader()
{
  if (!charlie::EMsg_IsValid(head.emsg()))
  {
    CERR("EMsg type " << head.emsg() << " is invalid.");
    return false;
  }

  // Validate handshake phase
  if (!handshakeComplete)
  {
    charlie::EMsg expected;
    // Expecting a CServerIdentify if !handshakeComplete and !receivedServerIdentify
    if (!receivedClientIdentify)
      expected = charlie::EMsgClientIdentify;
    else
      expected = charlie::EMsgClientAccept;

    if (expected != head.emsg())
    {
      CERR("Unexpected message " << head.emsg() << " during handshake phase.");
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
      CERR("Size for " << head.emsg() << " - " << head.body_size() << " bytes > expected maximum of " << maxSize << " bytes.");
      return false;
    }
  }

  // Validate the target module
  // XXX
  return true;
}

bool CharlieClient::handleClientIdentify(std::string& data)
{
  // Load up the message
  charlie::CClientIdentify ident;
  if (!ident.ParseFromString(data))
  {
    CERR("Unable to parse CClientIdentify from body.");
    return false;
  }

  if (!ident.has_client_pubkey() || !ident.has_client_challenge())
  {
    CERR("Client identify is missing challenge or pubkey.");
    return false;
  }

  if (sessionCrypto)
    delete sessionCrypto;

  sessionCrypto = new Crypto();
  if (sessionCrypto->setRemotePubKey((unsigned char*)ident.client_pubkey().c_str(), ident.client_pubkey().length()) != SUCCESS)
  {
    CERR("Unable to parse remote public key");
    return false;
  }

  // Check the challenge response
  if (sessionCrypto->digestVerify((const unsigned char*) serverChallenge.c_str(), serverChallenge.length(), (unsigned char*) ident.challenge_response().c_str(), ident.challenge_response().length()) != SUCCESS)
  {
    CERR("Challenge response isn't valid.");
    return false;
  }

  // Validate against this pubkey later
  clientChallenge = ident.client_challenge();
  if (strlen(clientChallenge.c_str()) != clientChallenge.length())
  {
    CERR("Strlen of clientChallenge not same as length of buffer");
    return false;
  }

  if (clientChallenge.length() != 10)
  {
    CERR("The clientChallenge is not 10. Invalid");
    return false;
  }

  CLOG("Received valid client identify.");
  receivedClientIdentify = true;
  sendServerAccept();

  return true;
}

void CharlieClient::handleClientAccept(std::string& data)
{
  charlie::CClientAccept acc;
  if (!acc.ParseFromString(data))
  {
    CERR("Unable to parse CClientAccept.");
    disconnect();
    return;
  }

  if (acc.has_system_info())
  {
    CLOG("Client cpu hash: " << acc.system_info().cpu_hash());
  }

  handshakeComplete = true;
  CLOG("Handshake complete.");
}

void CharlieClient::sendServerAccept()
{
  charlie::CServerAccept acc;
  acc.set_request_system_info(true);

  unsigned char* resp;
  int sigLen = serverCrypto->digestSign((const unsigned char*) clientChallenge.c_str(), clientChallenge.length(), &resp);
  acc.set_challenge_response(resp, sigLen);
  free(resp);

  std::string data = acc.SerializeAsString();
  send(charlie::EMsgServerAccept, 0, data);
}

bool CharlieClient::validateMessageBody()
{
  if (!body.has_signature())
  {
    CERR("Body is missing contents signature.");
    return false;
  }

  if (!body.has_timestamp_signature())
  {
    CERR("Body is missing timestamp signature.");
    return false;
  }

  if (!body.has_timestamp())
  {
    CERR("Body is missing timestamp.");
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
      CERR("Time skew of " << diff << " greater than " << ALLOWED_TIME_SKEW << ".");
      return false;
    }

    unsigned char* tsBuf = (unsigned char*) malloc(sizeof(unsigned char) * 4);
    tsBuf[0] = static_cast<unsigned char>((timestamp >> 24) & 0xFF);
    tsBuf[1] = static_cast<unsigned char>((timestamp >> 16) & 0xFF);
    tsBuf[2] = static_cast<unsigned char>((timestamp >> 8) & 0xFF);
    tsBuf[3] = static_cast<unsigned char>(timestamp & 0xFF);
    if (sessionCrypto->digestVerify((unsigned char*) tsBuf, 4, (unsigned char*) body.timestamp_signature().c_str(), body.timestamp_signature().length()) != SUCCESS)
    {
      CERR("Digest verification of timestamp incorrect.");
      return false;
    }
  }

  if (!body.has_rsa_body())
  {
    CERR("Body has no rsa body.");
    return false;
  }

  const std::string& contents = body.rsa_body().data();

  // Verify the signature
  if (sessionCrypto->digestVerify((const unsigned char*) contents.c_str(), contents.length(), (unsigned char*) body.signature().c_str(), body.signature().length(), true) != SUCCESS)
  {
    CERR("Incorrect signature for contents.");
    return false;
  }

  return true;
}

void CharlieClient::disconnect()
{
  host->clientDisconnected(shared_from_this());
  try {
    socket->close();
  } catch (std::exception& ex)
  {
    CERR("Error closing socket " << ex.what());
  }
}

CharlieClient::~CharlieClient()
{
  if (sessionCrypto)
    delete sessionCrypto;
}
