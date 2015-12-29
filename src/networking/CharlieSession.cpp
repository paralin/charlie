#include <networking/CharlieSession.h>
#include <charlie/random.h>
#include <Logging.h>

#define ALLOWED_TIME_SKEW 30

#ifdef MLOG
#define SLOG(msg) MLOG(msg)
#define SERR(msg) MERR(msg)
#else
#define SLOG(msg) CLOG(msg)
#define SERR(msg) CERR(msg)
#endif

CharlieSession::CharlieSession(std::shared_ptr<tcp::socket> socket, ISessionController* controller, std::string& priKey)
  : socket(socket),
  handshakeComplete(false),
  expectingHeader(false),
  receivedIdentify(false),
  expectingHeaderLengthPrefix(true),
  localChallenge(gen_random(10)),
  timeConnected(time(NULL)),
  lastMsg(time(NULL)),
  hasDisconnected(false)
{
  this->controller = controller;
  sessionCrypto = std::make_shared<Crypto>();
  sessionCrypto->setLocalPriKey((unsigned char*) priKey.c_str(), priKey.length());
  // init keepalive thread
}

void CharlieSession::start()
{
  auto self = this;
  SLOG("Starting session...");

#ifndef IS_CSERVER
  SLOG("Sending magic...");
  char mgc = CHARLIE_MAGIC;
  socket->send(boost::asio::buffer(&mgc, sizeof(char)));
  doRead();
#else
  // Read magic
  SLOG("Waiting for magic...");

  auto rBuf = boost::asio::buffer(&incMagic, sizeof(char));
  boost::asio::async_read(*socket,
      rBuf,
      [this, self](boost::system::error_code ec, std::size_t /*length*/)
      {
        if (ec)
        {
          SERR("Error code " << ec << " receiving magic.");
          disconnect();
          return;
        }
        if (incMagic != CHARLIE_MAGIC)
        {
          SERR("Magic "  << ((int)incMagic) << " != real magic " << ((int) CHARLIE_MAGIC));
          disconnect();
          return;
        }
        SLOG("Valid magic received, reading...");
        sendIdentify();
        doRead();
      });
#endif
}

void CharlieSession::send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data)
{
  SLOG("Sending routed message to target " << targetModule);
  charlie::CMessageTarget* target = new charlie::CMessageTarget();
  target->set_target_module(targetModule);
  target->set_emsg(targetEmsg);
  target->set_job_id(jobid);
  send(charlie::EMsgRoutedMessage, data, target);
}

void CharlieSession::send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target)
{
  if (hasDisconnected)
  {
    SERR("Already disconnected, dropping sent message.");
    return;
  }

  boost::unique_lock<boost::mutex> scoped_lock(sendMtx);
  charlie::CMessageHeader nHead;

  // Timestamp code
  {
    std::time_t now;
    std::time(&now);
    u32 timestamp = now - timeConnected;
    nHead.set_timestamp(timestamp);

    unsigned char tsBuf[4];
    tsBuf[0] = static_cast<unsigned char>((timestamp >> 24) & 0xFF);
    tsBuf[1] = static_cast<unsigned char>((timestamp >> 16) & 0xFF);
    tsBuf[2] = static_cast<unsigned char>((timestamp >> 8) & 0xFF);
    tsBuf[3] = static_cast<unsigned char>(timestamp & 0xFF);

    unsigned char* tsSigBuf;
    int digestLen = sessionCrypto->digestSign((const unsigned char*) tsBuf, sizeof(unsigned char) * 4, &tsSigBuf, false);
    nHead.set_timestamp_signature(tsSigBuf, digestLen);
    free(tsSigBuf);
  }

  std::string sendBuf;
#ifdef IS_CSERVER
  if (emsg == charlie::EMsgIdentify)
    sendBuf = data;
  else
#endif
  {
    charlie::CRSABuffer* buf = new charlie::CRSABuffer();
    encryptRsaBuf(buf, sessionCrypto.get(), (const unsigned char*) data.c_str(), data.length(), true);
    sendBuf = buf->SerializeAsString();
    delete buf;
  }

  {
    unsigned char* bodySigBuf;
    int digestLen = sessionCrypto->digestSign((const unsigned char*) data.c_str(), data.length(), &bodySigBuf, false);
    nHead.set_signature((const char*) bodySigBuf, digestLen);
    free(bodySigBuf);
  }

  nHead.set_emsg(emsg);
  if (target)
    nHead.set_allocated_target(target);
  nHead.set_body_size(sendBuf.length());

  std::string sHead = nHead.SerializeAsString();

  // Scramble these a bit to confuse people
  // Order is
  //  - 0 -> 2
  //  - 1 -> 0
  //  - 2 -> 3
  //  - 3 -> 1
  char hlenBuf[4];
  hlenBuf[2] = (static_cast<char>((sHead.length() >> 24) & 0xFF));
  hlenBuf[0] = (static_cast<char>((sHead.length() >> 16) & 0xFF));
  hlenBuf[3] = (static_cast<char>((sHead.length() >> 8) & 0xFF));
  hlenBuf[1] = (static_cast<char>(sHead.length() & 0xFF));

  SLOG("Sending message:\n  => emsg: " << nHead.emsg() << "\n  => body length: " << nHead.body_size() << "\n  => head length: " << sHead.length());
  try {
    boost::asio::write(*socket, boost::asio::buffer(hlenBuf));
    boost::asio::write(*socket, boost::asio::buffer(sHead));
    boost::asio::write(*socket, boost::asio::buffer(sendBuf));
  } catch (std::exception& ex)
  {
    SERR("Error sending to client " << ex.what() << " disconnecting.");
    disconnect();
    return;
  }
}

void CharlieSession::doKeepalive()
{
  while (!hasDisconnected)
  {
    if (handshakeComplete)
    {
      std::time_t now = time(NULL);
      if ((now - lastMsg) > 30)
      {
        SERR("Haven't received anything in 30 seconds, assuming disconnect.");
        disconnect();
        return;
      }
      charlie::CKeepAlive aliv;
      std::string d = aliv.SerializeAsString();
      send(charlie::EMsgKeepalive, d);
    }
    boost::this_thread::sleep(boost::posix_time::milliseconds(10000));
  }
}

void CharlieSession::doRead()
{
  auto self = this;

  if (!expectingHeaderLengthPrefix)
  {
    u32 bufSize;

    if (expectingHeader)
      bufSize = expectedHeaderSize;
    else
      bufSize = expectedBodySize;

    if (bufSize == 0)
    {
      buffer.clear();
      handleReadData();
      return;
    }
    buffer.resize(bufSize);
  }

  auto rBuf = expectingHeaderLengthPrefix ?
    boost::asio::buffer(headerLengthPrefixBuf)
    : boost::asio::buffer(buffer);
  boost::asio::async_read(*socket,
      rBuf,
      [this, self](boost::system::error_code ec, std::size_t /*length*/)
      {
        if (!ec)
          handleReadData();
        else
        {
          SERR("Error code " << ec << " receiving.");
          disconnect();
        }
      });
}

void CharlieSession::handleReadData()
{
  auto self = this;
  std::time(&lastMsg);

  if (expectingHeaderLengthPrefix)
  {
    SLOG("Received 4 byte header prefix.");
  }
  else
    SLOG("Received " << buffer.size() << " bytes.");

  if (expectingHeaderLengthPrefix)
  {
    expectingHeaderLengthPrefix = false;
    expectingHeader = true;
    expectedHeaderSize = 0;

    // Order is
    //  - 0 -> 2
    //  - 1 -> 0
    //  - 2 -> 3
    //  - 3 -> 1
    // Also each is xor by the number of messages sent
    expectedHeaderSize = expectedHeaderSize * 256 + (static_cast<char>(headerLengthPrefixBuf[2]) & 0XFF);
    expectedHeaderSize = expectedHeaderSize * 256 + (static_cast<char>(headerLengthPrefixBuf[0]) & 0XFF);
    expectedHeaderSize = expectedHeaderSize * 256 + (static_cast<char>(headerLengthPrefixBuf[3]) & 0XFF);
    expectedHeaderSize = expectedHeaderSize * 256 + (static_cast<char>(headerLengthPrefixBuf[1]) & 0XFF);

    if (expectedHeaderSize > 800)
    {
      SERR("Received header size of " << expectedHeaderSize << ", too big.");
      disconnect();
      return;
    }
  }
  else if (expectingHeader)
  {
    head.Clear();
    if (!head.ParseFromArray(&buffer[0], buffer.size()))
    {
      SERR("CMessageHeader parse failed.");
      disconnect();
      return;
    }

    if (!validateMessageHeader())
    {
      SERR("Messgae header failed to validate.");
      disconnect();
      return;
    }

    expectingHeader = false;
    expectedBodySize = head.body_size();
  } else
  {
#ifndef IS_CSERVER
    if (!handshakeComplete && head.emsg() == charlie::EMsgIdentify)
    {
      SLOG("Skipping message validation as message is identify.");
      //handleHandshakeMessage();
    } else
#endif
    if (!validateMessageBody())
    {
      SERR("Body verification failed.");
      disconnect();
      return;
    }

    SLOG("Received message:\n  => emsg: " << head.emsg() << "\n  => body length: " << head.body_size() << "\n  => head length: " << head.ByteSize());

    expectingHeaderLengthPrefix = true;
    expectingHeader = false;
    expectedBodySize = 0;
    expectedHeaderSize = 0;

    if (head.emsg() == charlie::EMsgKeepalive)
      time(&lastMsg);
    else if (!handshakeComplete)
      handleHandshakeMessage();
    else
      controller->handleMessage(head, decryptedData);
  }

  doRead();
}

bool CharlieSession::validateMessageHeader()
{
  if (!charlie::EMsg_IsValid(head.emsg()))
  {
    SERR("EMsg type " << head.emsg() << " is invalid.");
    return false;
  }

  // Validate handshake phase
  if (!handshakeComplete)
  {
    charlie::EMsg expected;
    if (!receivedIdentify)
      expected = charlie::EMsgIdentify;
    else
      expected = charlie::EMsgAccept;

    if (expected != head.emsg())
    {
      SERR("Unexpected message " << head.emsg() << " during handshake phase.");
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
      SERR("Size for " << head.emsg() << " - " << head.body_size() << " bytes > expected maximum of " << maxSize << " bytes.");
      return false;
    }
  }

  if (!head.has_signature())
  {
    SERR("Head is missing contents signature.");
    return false;
  }

  if (!head.has_timestamp_signature())
  {
    SERR("Head is missing timestamp signature.");
    return false;
  }

  if (!head.has_timestamp())
  {
    SERR("Head is missing timestamp.");
    return false;
  }
  return true;
}

bool CharlieSession::validateMessageBody()
{
#ifdef IS_CSERVER
  if (head.emsg() == charlie::EMsgIdentify)
    return decryptBody();
#endif

  {
    std::time_t now;
    std::time(&now);
    now -= timeConnected;
    std::time_t timestamp = head.timestamp();
    std::time_t diff = std::abs(now - timestamp);
    if (diff > ALLOWED_TIME_SKEW)
    {
      SERR("Time skew of " << diff << " greater than " << ALLOWED_TIME_SKEW << ".");
      return false;
    }

    unsigned char tsBuf[4];
    tsBuf[0] = static_cast<unsigned char>((timestamp >> 24) & 0xFF);
    tsBuf[1] = static_cast<unsigned char>((timestamp >> 16) & 0xFF);
    tsBuf[2] = static_cast<unsigned char>((timestamp >> 8) & 0xFF);
    tsBuf[3] = static_cast<unsigned char>(timestamp & 0xFF);
    if (sessionCrypto->digestVerify((const unsigned char*) tsBuf, sizeof(unsigned char) * 4, (unsigned char*) head.timestamp_signature().c_str(), head.timestamp_signature().length()) != SUCCESS)
    {
      SERR("Digest verification of timestamp " << timestamp << " incorrect.");
      return false;
    }
  }

  if (!decryptBody())
    return false;

  // Verify the signature
  if (sessionCrypto->digestVerify((const unsigned char*) decryptedData.c_str(), decryptedData.length(), (unsigned char*) head.signature().c_str(), head.signature().length(), true) != SUCCESS)
  {
    SERR("Incorrect signature for contents.");
    return false;
  }

  return true;
}

bool CharlieSession::decryptBody()
{
  charlie::CRSABuffer rbuf;
  if (!rbuf.ParseFromArray(&buffer[0], buffer.size()))
  {
    SERR("Unable to parse CRSABuffer.");
    return false;
  }

  unsigned char* decrypted = NULL;
  int len = decryptRsaBuf(&rbuf, sessionCrypto.get(), &decrypted, false);
  if (len == FAILURE)
  {
    SERR("Unable to decrypt rsa buffer.");
    if (decrypted == NULL)
      free(decrypted);
    return false;
  }
  decryptedData = std::string((char*) decrypted, len);
  free(decrypted);
  return true;
}

void CharlieSession::handleHandshakeMessage()
{
  if (head.emsg() == charlie::EMsgIdentify)
    handleIdentify();
  else if (head.emsg() == charlie::EMsgAccept)
    handleAccept();
}

void CharlieSession::handleIdentify()
{
  SLOG("Received remote identify.");
#ifndef IS_CSERVER
  decryptedData = std::string(&buffer[0], buffer.size());
#endif
  charlie::CNetIdentify ident;
  if (!ident.ParseFromString(decryptedData))
  {
    SERR("Unable to parse identify.");
    disconnect();
    return;
  }

  if (!ident.has_pubkey())
  {
    SERR("Identify doesn't have pubkey.");
    disconnect();
    return;
  }

  if (!ident.has_challenge())
  {
    SERR("Identify doesn't have challenge.");
    disconnect();
    return;
  }

  if (sessionCrypto->setRemotePubKey((unsigned char*) ident.pubkey().c_str(), ident.pubkey().length()) != SUCCESS)
  {
    SERR("Unable to set remote pubkey.");
    SERR(ident.pubkey());
    disconnect();
    return;
  }

#if IS_CSERVER
  if (!ident.has_challenge_response())
  {
    SERR("Client identify doesn't have challenge response.");
    disconnect();
    return;
  }

  // Check the challenge response
  if (sessionCrypto->digestVerify((const unsigned char*) localChallenge.c_str(), localChallenge.length(), (unsigned char*) ident.challenge_response().c_str(), ident.challenge_response().length()) != SUCCESS)
  {
    CERR("Challenge response isn't valid.");
    disconnect();
    return;
  }
#endif

  remoteChallenge = ident.challenge();

  receivedIdentify = true;
#ifndef IS_CSERVER
  sendIdentify();
#else
  sendAccept();
#endif
}

void CharlieSession::sendIdentify()
{
  charlie::CNetIdentify ident;
  unsigned char* pkey;
  int plen = sessionCrypto->getLocalPubKey(&pkey);
  ident.set_pubkey(pkey, plen);
  free(pkey);
  ident.set_challenge(localChallenge);
#ifndef IS_CSERVER
  // Create the challenge response
  unsigned char* resp = NULL;
  size_t len = sessionCrypto->digestSign((const unsigned char*) remoteChallenge.c_str(), remoteChallenge.length(), &resp, false);
  if (len == FAILURE)
  {
    SERR("Digest sign not successful!");
    disconnect();
    return;
  }
  ident.set_challenge_response(resp, len);
  free(resp);
#endif
  std::string data = ident.SerializeAsString();
  SLOG("Sending identify, length " << data.length());
  send(charlie::EMsgIdentify, data);
}

void CharlieSession::handleAccept()
{
  SLOG("Received remote accept.");
  charlie::CNetAccept acc;
  if (!acc.ParseFromString(decryptedData))
  {
    SERR("Unable to parse remote accept.");
    disconnect();
    return;
  }

  handshakeComplete = true;
  controller->onHandshakeComplete();
#ifndef IS_CSERVER
  // Check the challenge response
  if (sessionCrypto->digestVerify((const unsigned char*) localChallenge.c_str(), localChallenge.length(), (unsigned char*) acc.challenge_response().c_str(), acc.challenge_response().length()) != SUCCESS)
  {
    SERR("Challenge response isn't valid.");
    disconnect();
    return;
  }

  sendAccept();
#endif
}

void CharlieSession::setController(ISessionController* cont)
{
  controller = cont;
}

void CharlieSession::sendAccept()
{
  charlie::CNetAccept acc;
#ifdef IS_CSERVER
  // Create the challenge response
  unsigned char* resp = NULL;
  size_t len = sessionCrypto->digestSign((const unsigned char*) remoteChallenge.c_str(), remoteChallenge.length(), &resp, false);
  if (len == FAILURE)
  {
    SERR("Digest sign not successful!");
    disconnect();
    return;
  }
  acc.set_challenge_response(resp, len);
  free(resp);
#endif

  std::string data = acc.SerializeAsString();
  send(charlie::EMsgAccept, data);
}

void CharlieSession::disconnect()
{
  hasDisconnected = true;
  controller->onDisconnected();
}

CharlieSession::~CharlieSession()
{
  SLOG("Deconstructing session...");
  hasDisconnected = true;
}
