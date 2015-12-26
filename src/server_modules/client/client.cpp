#include <server_modules/client/Client.h>
#include <protogen/client.pb.h>
#include <boost/thread.hpp>

using namespace server_modules::client;
using namespace modules::client;

ClientModule::ClientModule() :
  sessionCrypto(NULL),
  systemCrypto(NULL),
  mInter(NULL)
{
  MLOG("Client module constructed...");
}

void ClientModule::shutdown()
{
}

void ClientModule::setModuleInterface(ServerModuleInterface* inter)
{
  mInter = inter;
}

void ClientModule::inject(u32 id, void* dep)
{
}

void ClientModule::release(u32 id)
{
}

void ClientModule::handleEvent(u32 event, void* data)
{
}

void ClientModule::module_main()
{
  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
  // First thing's first, request some info.
  MLOG("Requesting client info...");
  CClientRequestSystemInfo nfo;
  std::string data = nfo.SerializeAsString();
  mInter->send(getModuleId(), EClientEMsg_RequestSystemInfo, 0, data);
}

void ClientModule::handleMessage(charlie::CMessageTarget* target, std::string& data)
{
  if (target->emsg() == EClientEMsg_SystemInfo)
  {
    MLOG("Received client information.");
    CClientSystemInfo cinfo;
    if (!cinfo.ParseFromString(data))
    {
      MERR("Client info does not parse properly.");
      return;
    }

    MLOG("SystemID: " << cinfo.system_id());
    MLOG("CPUHash:  " << cinfo.cpu_hash());
    MLOG("Hostname: " << cinfo.hostname());
    return;
  }

  MERR("Unknown message received: " << target->emsg());
}

u32 ClientModule::getModuleId()
{
  return 2777954855;
}

ClientModule::~ClientModule()
{
}


CHARLIE_CONSTRUCT(ClientModule);
