#include <server_modules/client/Client.h>

using namespace server_modules::client;

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
}

ClientModule::~ClientModule()
{
}


CHARLIE_CONSTRUCT(ClientModule);
