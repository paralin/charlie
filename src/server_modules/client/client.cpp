#include <server_modules/client/Client.h>
#include <protogen/client.pb.h>
#include <boost/thread.hpp>

using namespace server_modules::client;
using namespace modules::client;

ClientModule::ClientModule() :
  sessionCrypto(NULL),
  systemCrypto(NULL),
  mInter(NULL),
  clientInfoReceived(false),
  clientModulesReceived(false),
  isShutdown(false)
{
  MLOG("Client module constructed...");
}

void ClientModule::shutdown()
{
  isShutdown = true;
}

void ClientModule::setModuleInterface(SModuleInterface* inter)
{
  mInter = inter;
}

void ClientModule::inject(u32 id, void* dep)
{
  if (id == 210060544)
  {
    MLOG("Got mongo interface.");
    mongoMod = (mongo::MongoModule*) dep;
  }
}

void ClientModule::release(u32 id)
{
  if (id == 210060544)
  {
    MLOG("Lost mongo interface.");
    mongoMod = NULL;
  }
}

void ClientModule::handleEvent(u32 event, void* data)
{
}

void ClientModule::module_main()
{
  boost::this_thread::sleep(boost::posix_time::milliseconds(1000));
  // First thing's first, request some info.
  MLOG("Requesting client info...");
  SEND_EMPTYMSG(CClientRequestSystemInfo, EClientEMsg_RequestSystemInfo);

  std::time_t clientInfoTimeout = std::time(NULL) + 30;
  while (!clientInfoReceived || !clientModulesReceived)
  {
    std::time_t now = std::time(NULL);
    if (isShutdown)
      return;
    if (now > clientInfoTimeout)
    {
      MERR("Client info timeout.");
      mInter->disconnect();
      return;
    }
  }
}

void ClientModule::handleMessage(charlie::CMessageTarget* target, std::string& data)
{
  if (target->emsg() == EClientEMsg_SystemInfo)
  {
    MLOG("Received client information.");
    clientInfoReceived = true;
    CClientSystemInfo cinfo;
    if (!cinfo.ParseFromString(data))
    {
      MERR("Client info does not parse properly.");
      return;
    }

    MLOG("SystemID: " << cinfo.system_id());
    MLOG("CPUHash:  " << cinfo.cpu_hash());
    MLOG("Hostname: " << cinfo.hostname());

    if (mongoMod)
    {
      mongoMod->initWithInfo(cinfo);
      SEND_EMPTYMSG(CClientRequestModuleState, EClientEMsg_RequestModuleState);
    }

    return;
  }

  if (target->emsg() == EClientEMsg_ModuleState)
  {
    if (!clientInfoReceived)
      return;

    MLOG("Received client module list.");
    clientModulesReceived = true;
    CClientModuleState cinfo;
    if (!cinfo.ParseFromString(data))
    {
      MERR("Client modules list does not parse properly.");
      return;
    }

    MLOG("-- received module list --");
    for (int i = 0; i < cinfo.modules_size(); i++)
    {
      const charlie::CModuleInstance& m = cinfo.modules(i);
      MLOG(" - " << m.id() << ": " << m.status());
    }

    if (mongoMod)
      mongoMod->updateModuleStates(cinfo);

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
