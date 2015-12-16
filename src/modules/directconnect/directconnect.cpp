#include <Module.h>
#include <modules/directconnect/DirectConnect.h>

using namespace modules::directconnect;

DirectConnectModule::DirectConnectModule()
{
  MLOG("Direct connect socket net module constructed...");
  pInter = new DirectConnectInter(this);
  io_service = NULL;
  resolver = NULL;
  socket = NULL;
  running = true;
  connected = false;
  wasConnected = false;
  expectingHeaderLengthPrefix = false;
  expectingHeader = false;
  expectedBodySize = 0;
}

DirectConnectModule::~DirectConnectModule()
{
  delete pInter;
  releaseNetworking();
}

void DirectConnectModule::releaseNetworking()
{
  if (io_service)
  {
    delete io_service;
    io_service = NULL;
  }
  if (resolver)
  {
    delete resolver;
    resolver = NULL;
  }
  disconnect();
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

void DirectConnectModule::resetReceiveContext()
{
  expectingHeaderLengthPrefix = true;
  expectingHeader = false;
  expectedBodySize = 0;
}

void DirectConnectModule::unexpectedDataReceived()
{
  MLOG("Unexpected data received, disconnect.");
  disconnect();
  resetReceiveContext();
}

// Main function
void DirectConnectModule::module_main()
{
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
          disconnect();
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

      std::vector<unsigned char> buf(bufSize);
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
            expectedHeaderSize = expectedHeaderSize * 256 + (static_cast<u32>(buf[i]) & 0xFF);
        // 80 is more than the expected size
        if (expectedHeaderSize > 80)
        {
          unexpectedDataReceived();
          continue;
        }
      } else if (expectingHeader)
      {
        charlie::CMessageHeader head;
        if (!head.ParseFromArray(&buf[0], buf.size()))
        {
          MERR("CMessageHeader parse failed");
          unexpectedDataReceived();
          continue;
        }

        // Validate the message header
      }
    }
  } catch (std::exception& ex)
  {
    MLOG("Caught overall exception " << ex.what());
  }

  MLOG("Releasing networking...");
  releaseNetworking();
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
