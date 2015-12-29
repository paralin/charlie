#define MODULE_ID 6032034
#include <modules/torm/Torm.h>
#include <modules/torm/TorC.h>
#include <modules/torm/socks5.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <charlie/random.h>
#include <charlie/hash.h>
#include <modules/manager/ManagerInter.h>
#include <networking/CharlieSession.h>
#include <chrono>

typedef std::chrono::high_resolution_clock Clock;

#define DEBUGGING_TORC

using namespace modules::torm;

TormModule::TormModule() :
  mInter(NULL),
  pInter(new TormInter(this)),
  running(true),
  inited(false),
  manager(NULL),
  client(NULL),
  io_service(),
  resolver(io_service)
{
  MLOG("Torm module constructed...");
}

TormModule::~TormModule()
{
  delete pInter;
}

void TormModule::shutdown()
{
  MLOG("Shutting down torm module..");
  running = false;
  torc_shutdown();
}

void TormModule::setModuleInterface(ModuleInterface* inter)
{
  mInter = inter;
}

void* TormModule::getPublicInterface()
{
  return pInter;
}

void TormModule::injectDependency(u32 id, void* dep)
{
  MLOG("Dep injected " << id);
  if (id == MANAGER_MODULE_ID)
    manager = (modules::manager::ManagerInter*) dep;
  if (id == CLIENT_MODULE_ID)
    client = (modules::client::ClientInter*) dep;
}

void TormModule::releaseDependency(u32 id)
{
  if (id == MANAGER_MODULE_ID)
    manager = NULL;
  if (id == CLIENT_MODULE_ID)
    client = NULL;
}

bool TormModule::parseModuleInfo()
{
  std::string info = mInter->getModuleInfo();
  return sInfo.ParseFromString(info) == 0;
}

void TormModule::module_main()
{
  parseModuleInfo();
  std::string dataDir;
  mInter->requireDependency(MANAGER_MODULE_ID, false);
  mInter->requireDependency(CLIENT_MODULE_ID, false);
  {
    SystemInfo *info = mInter->getSysInfo();
    torPort = info->lock_port + 1 + (rand() % 100);
    dataDir = info->root_path;
    dataDir += "or";
  }
  MLOG("Data dir: " << dataDir);
  socksUsername = gen_random(rand()%(15-5)+5);
  socksPassword = gen_random(rand()%(15-5)+5);
  connectControlLoop = boost::thread(&TormModule::connectControl, this);
  inited = true;
  torc_main(torPort, socksUsername.c_str(), socksPassword.c_str(), dataDir.c_str());
}

void TormModule::connectControl()
{
  MLOG("Starting connect control loop...");
  bool justCompleted = false;
  bool hasGivenManagerProxy = false;
  while (running)
  {
    if (!have_completed_a_circuit())
    {
      MLOG("Waiting for Tor to finish starting up...");
      justCompleted = true;
      socket.reset();
      session.reset();
      boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
      continue;
    }
    if (justCompleted)
    {
      MLOG("Tor became ready, attempting connections...");
      justCompleted = false;
    }
    if (manager == NULL)
    {
      hasGivenManagerProxy = false;
    } else if (!hasGivenManagerProxy)
    {
#ifdef GIVE_MANAGER_PROXY
      manager->setOrProxy(std::string("socks5h://127.0.0.1:") + std::to_string(torPort), socksUsername + ":" + socksPassword);
#endif
      hasGivenManagerProxy = true;
    }
    if (session)
    {
      auto t1 = Clock::now();
      io_service.poll_one();
      auto t2 = Clock::now();
      auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();
      if (ms < 50)
        boost::this_thread::sleep(boost::posix_time::milliseconds(50 - ms));
      continue;
    }
    if (!tryConnectAllEndpoints())
    {
      // Connection unsuccessful
      boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
    }
  }
}

bool TormModule::tryConnectAllEndpoints()
{
  std::time_t now;
  time(&now);
  for (int i = 0; i < sInfo.endpoints_size(); i++)
  {
    std::string endp = sInfo.endpoints(i);
    unsigned int endphash = hashString(endp.c_str());
    if (endpointTimeouts.count(endphash))
    {
      if (endpointTimeouts[endphash] > now)
        continue;
      endpointTimeouts.erase(endphash);
    }
    MLOG("Attempting to connect to " << endp << "...");
    if (tryConnectEndpoint(endp))
    {
      MLOG("Connection successful...");
      unsigned char* key;
      int klen = mInter->getCrypto()->getLocalPriKey(&key);
      std::string ke((char*) key, klen);
      free(key);
      session = std::make_shared<CharlieSession>(socket, this, ke);
      session->start();
      socket.reset();
      MLOG("Notifying client we're connected.");
      if (client)
        client->retryConnectionsNow();
      return true;
    }
  }
  return false;
}

bool TormModule::tryConnectEndpoint(std::string& endp)
{
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

  int dport = boost::lexical_cast<int>(port);

  MLOG("Establishing socks connection...");
  if (!tryEstablishSocksConnection())
  {
    MLOG("Unable to establish socks connection...");
    return false;
  }

  MLOG("Attempting socks negotation to " << ipadd << " port: " << dport);
  {
    socks5::socks5_req connReq;
    connReq.Version = socks5::version;
    // Tcp connection
    connReq.Cmd = 0x01;
    connReq.Reserved = 0x0;
    // Assume onion routing
    connReq.AddrType = 0x03;
    socket->send(boost::asio::buffer(&connReq, sizeof(socks5::socks5_req)));
    unsigned char ipaddlen = ipadd.length();
    socket->send(boost::asio::buffer(&ipaddlen, sizeof(unsigned char)));
    socket->send(boost::asio::buffer(ipadd.data(), ipadd.length()));
    unsigned char dportb[2];
    dportb[0] = (unsigned char)((dport >> 8) & 0xff);
    dportb[1] = (unsigned char)(dport & 0xff);
    socket->send(boost::asio::buffer(&dportb, 2));
  }
  {
    socks5::socks5_resp cresp;
    socket->receive(boost::asio::buffer(&cresp, sizeof(socks5::socks5_resp)));
    if (cresp.Version != socks5::version)
    {
      MERR("Version in connect response is wrong.");
      return false;
    }
    if (cresp.Reply != 0x00)
    {
      MERR("Request was not granted, reply: " << ((int)cresp.Reply));
      handleEndpointFailure(hashString(endp.c_str()), cresp.Reply);
      return false;
    }
    if (cresp.AddrType == 0x03)
    {
      unsigned char dnlen;
      char* domainName;
      socket->receive(boost::asio::buffer(&dnlen, sizeof(unsigned char)));
      if (dnlen > 80 || dnlen < 2)
      {
        MERR("Domain name len is " << ((int)dnlen) << " which is suspicious.");
        return false;
      }
      domainName = (char*) malloc(sizeof(char) * dnlen);
      socket->receive(boost::asio::buffer(domainName, dnlen));
      MLOG("Confirmed domain is " << domainName);
      free(domainName);
    }
    else if (cresp.AddrType == 0x01)
    {
      unsigned char ipv4b[4];
      socket->receive(boost::asio::buffer(&ipv4b, 4));
    }
    else if (cresp.AddrType == 0x04)
    {
      unsigned char ipv6b[16];
      socket->receive(boost::asio::buffer(&ipv6b, 16));
    } else
    {
      MLOG("Unknown response address type " << ((int)cresp.AddrType));
    }
    unsigned char portdb[2];
    socket->receive(boost::asio::buffer(&portdb, 2));
    // MLOG("Confirmed port is " << port);
  }
  return true;
}

void TormModule::handleEndpointFailure(unsigned int endpHash, unsigned char err)
{
  std::time_t now;
  time(&now);
  switch (err)
  {
    case 0x01:
#ifndef DEBUGGING_TORC
      MERR("General endpoint failure, will try again immediately.");
#else
      MERR("General endpoint failure, will try again in 60 seconds.");
      endpointTimeouts[endpHash] = now + 60;
#endif
      break;
    case 0x02:
      MERR("Endpoint connection apparently not allowed, timing out significantly.");
      endpointTimeouts[endpHash] = now + 30;
      break;
    case 0x03:
      MERR("Network unreachable, not an endpoint problem.");
      endpointTimeouts[endpHash] = now + 10;
      break;
    case 0x04:
      MERR("Endpoint unreachable, trying again later.");
      endpointTimeouts[endpHash] = now + 120;
      break;
    case 0x05:
      MERR("Connection refused, server probably down.");
      // Time out by 30 seconds
      endpointTimeouts[endpHash] = now + 30;
      break;
    default:
      MERR("Unknown failure.");
      endpointTimeouts[endpHash] = now + 10;
      break;
  }
}

std::shared_ptr<CharlieSession> TormModule::getSession()
{
  MLOG("Returning session.");
  return session;
}

bool TormModule::tryEstablishSocksConnection()
{
  std::string localhost("127.0.0.1");
  std::string port = std::to_string(torPort);

  tcp::resolver::query query(localhost, port, boost::asio::ip::resolver_query_base::numeric_service);
  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  socket = std::make_shared<tcp::socket>(io_service);
  try {
    boost::asio::connect(*socket, endpoint_iterator);
  }
  catch (const boost::system::error_code& ex)
  {
    MERR("Unable to connect to tor proxy, error: " << ex);
    return false;
  }

  MLOG("Connection to socks established, authenticating...");

  try
  {
    socks5::socks5_ident_req ireq;
    ireq.Version = socks5::version;
    ireq.NumberOfMethods = 2;
    ireq.Methods[0] = 0x00;
    ireq.Methods[1] = 0x02;
    socket->send(boost::asio::buffer(&ireq, sizeof(socks5::socks5_ident_req)));
    socks5::socks5_ident_resp iresp;
    socket->receive(boost::asio::buffer(&iresp, sizeof(iresp)));
    if (iresp.Version != socks5::version)
    {
      MERR("Socks server version " << ((int) iresp.Version) << " incorrect.");
      return false;
    }
    if (iresp.Method == 0x00)
    {
      MLOG("Proceeding with no authentication...");
    }
    else if (iresp.Method == 0x02)
    {
      MLOG("Negotating username/password.");
      char negotiateBuf[2];
      // version
      negotiateBuf[0] = (char) 0x01;
      // length of name
      negotiateBuf[1] = (char) socksUsername.length();
      socket->send(boost::asio::buffer(negotiateBuf, 2));
      // name
      socket->send(boost::asio::buffer(socksUsername.c_str(), socksUsername.length()));
      // length of password
      char pwlen = socksPassword.length();
      socket->send(boost::asio::buffer(&pwlen, sizeof(char)));
      // password
      socket->send(boost::asio::buffer(socksPassword.c_str(), socksPassword.length()));
      socks5::socks5_generic_response aresp;
      socket->receive(boost::asio::buffer(&aresp, sizeof(socks5::socks5_generic_response)));
      if (aresp.Version != 0x01)
      {
        MERR("Socks version wrong in auth response, assuming error.");
        return false;
      }
      if (aresp.Status != 0x0)
      {
        MERR("Auth response is " << aresp.Status << ", assuming failure.");
        return false;
      }
    }
    else
    {
      MERR("Socks negotation method " << ((int) iresp.Method) << " not supported.");
      return false;
    }
  }
  catch (const boost::system::error_code& ex)
  {
    MERR("Error " << ex << " during negotation.");
    return false;
  }
  return true;
}

void TormModule::handleMessage(charlie::CMessageHeader& head, std::string& body)
{
  MERR("Unhandled message " << head.emsg() << ", client hasn't set handler yet.");
}

void TormModule::onDisconnected()
{
  MERR("Disconnected...");
  session.reset();
  socket.reset();
}

void TormModule::onHandshakeComplete()
{
  MLOG("Handshake complete.");
}

void TormModule::handleEvent(u32 eve, void* data)
{
}

void TormModule::newIdentity()
{
  session.reset();
  socket.reset();
  if (!running || !inited)
    return;

  MLOG("Requesting new identity...");
  torc_new_identity();
}

TormInter::TormInter(TormModule * mod)
{
  this->mod = mod;
}

bool TormInter::ready()
{
  return (bool) mod->session;
}

void TormInter::disconnectInvalid()
{
  MLOG("Disconnect invalid called.");
  mod->newIdentity();
}

std::shared_ptr<CharlieSession> TormInter::getSession()
{
  return mod->getSession();
}

TormInter::~TormInter()
{
}

void TormInter::handleCommand(const charlie::CMessageTarget& targ, std::string& command)
{
  MERR("Don't know how to handle emsg " << targ.emsg() << ".");
}

CHARLIE_CONSTRUCT(TormModule);
