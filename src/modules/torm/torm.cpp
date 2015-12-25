#define MODULE_ID 6032034
#include <modules/torm/Torm.h>
#include <modules/torm/TorC.h>
#include <modules/torm/socks5.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/thread/thread.hpp>
#include <charlie/random.h>
#include <charlie/hash.h>
#include <modules/manager/ManagerInter.h>

using namespace modules::torm;

TormModule::TormModule() :
  mInter(NULL),
  pInter(new TormInter(this)),
  running(true),
  manager(NULL),
  io_service(),
  resolver(io_service),
  socket(io_service)
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
  if (id == MANAGER_MODULE_ID)
    manager = (modules::manager::ManagerInter*) dep;
}

void TormModule::releaseDependency(u32 id)
{
  if (id == MANAGER_MODULE_ID)
    manager = NULL;
}

bool TormModule::parseModuleInfo()
{
  std::string info = mInter->getModuleInfo();
  return sInfo.ParseFromString(info) == 0;
}

void TormModule::module_main()
{
  parseModuleInfo();
  mInter->requireDependency(MANAGER_MODULE_ID, false);
  {
    SystemInfo *info = mInter->getSysInfo();
    torPort = info->lock_port + 1 + (rand() % 100);
  }
  socksUsername = gen_random(rand()%(15-5)+5);
  socksPassword = gen_random(rand()%(15-5)+5);
  connectControlLoop = boost::thread(&TormModule::connectControl, this);
  torc_main(torPort, socksUsername.c_str(), socksPassword.c_str());
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
      connected = false;
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
      manager->setOrProxy(std::string("socks5h://127.0.0.1:") + std::to_string(torPort), socksUsername + ":" + socksPassword);
    }
    if (connected)
    {
      boost::this_thread::sleep(boost::posix_time::milliseconds(200));
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
      std::time(&timeConnected);
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

  unsigned short dport = boost::lexical_cast<unsigned short>(port);

  MLOG("Establishing socks connection...");
  if (!tryEstablishSocksConnection())
  {
    MLOG("Unable to establish socks connection...");
    return false;
  }

  MLOG("Attempting socks negotation to " << ipadd << " port: " << port);
  {
    socks5::socks5_req connReq;
    connReq.Version = socks5::version;
    // Tcp connection
    connReq.Cmd = 0x01;
    connReq.Reserved = 0x0;
    // Assume onion routing
    connReq.AddrType = 0x03;
    socket.send(boost::asio::buffer(&connReq, sizeof(socks5::socks5_req)));
    unsigned char ipaddlen = ipadd.length();
    socket.send(boost::asio::buffer(&ipaddlen, sizeof(unsigned char)));
    socket.send(boost::asio::buffer(ipadd.data(), ipadd.length()));
    socket.send(boost::asio::buffer(&dport, sizeof(unsigned short)));
  }
  {
    socks5::socks5_resp cresp;
    socket.receive(boost::asio::buffer(&cresp, sizeof(socks5::socks5_resp)));
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
    if (cresp.AddrType != 0x03)
    {
      MERR("Address type in response is " << cresp.AddrType << " which isn't the expected.");
      return false;
    }
    unsigned char dnlen;
    char* domainName;
    socket.receive(boost::asio::buffer(&dnlen, sizeof(unsigned char)));
    if (dnlen > 80 || dnlen < 2)
    {
      MERR("Domain name len is " << dnlen << " which is suspicious.");
      return false;
    }
    domainName = (char*) malloc(sizeof(char) * dnlen);
    socket.receive(boost::asio::buffer(domainName, dnlen));
    MLOG("Confirmed domain is " << domainName);
    unsigned short port;
    socket.receive(boost::asio::buffer(&port, sizeof(unsigned char)));
    MLOG("Confirmed port is " << port);
    free(domainName);
  }
  connected = true;
  return true;
}

void TormModule::handleEndpointFailure(unsigned int endpHash, unsigned char err)
{
  std::time_t now;
  time(&now);
  switch (err)
  {
    case 0x01:
      MERR("General endpoint failure, will try again immediately.");
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

bool TormModule::tryEstablishSocksConnection()
{
  std::string localhost("127.0.0.1");
  std::string port = std::to_string(torPort);

  tcp::resolver::query query(localhost, port, boost::asio::ip::resolver_query_base::numeric_service);
  tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  socket = boost::asio::ip::tcp::socket(io_service);
  try {
    boost::asio::connect(socket, endpoint_iterator);
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
    socket.send(boost::asio::buffer(&ireq, sizeof(socks5::socks5_ident_req)));
    socks5::socks5_ident_resp iresp;
    socket.receive(boost::asio::buffer(&iresp, sizeof(iresp)));
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
      socket.send(boost::asio::buffer(negotiateBuf, 2));
      // name
      socket.send(boost::asio::buffer(socksUsername.c_str(), socksUsername.length()));
      // length of password
      char pwlen = socksPassword.length();
      socket.send(boost::asio::buffer(&pwlen, sizeof(char)));
      // password
      socket.send(boost::asio::buffer(socksPassword.c_str(), socksPassword.length()));
      socks5::socks5_generic_response aresp;
      socket.receive(boost::asio::buffer(&aresp, sizeof(socks5::socks5_generic_response)));
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

void TormModule::handleEvent(u32 eve, void* data)
{
}

void TormModule::newIdentity()
{
  if (!running)
    return;

  MLOG("Requesting new identity...");
  connected = false;
  torc_new_identity();
}

TormInter::TormInter(TormModule * mod)
{
  this->mod = mod;
}

bool TormInter::ready()
{
  return mod->connected;
}

void TormInter::disconnectInvalid()
{
  mod->connected = false;
  mod->newIdentity();
}

tcp::socket* TormInter::getSocket()
{
  return mod->getSocket();
}

TormInter::~TormInter()
{
}

void TormInter::handleCommand(const charlie::CMessageTarget& targ, std::string& command)
{
  MERR("Don't know how to handle emsg " << targ.emsg() << ".");
}

CHARLIE_CONSTRUCT(TormModule);
