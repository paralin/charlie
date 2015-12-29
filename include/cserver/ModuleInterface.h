#pragma once

#include <charlie/Crypto.h>
#include <SModuleInterface.h>

class CharlieClient;
namespace server_modules
{
  class ServerModuleInterface : public SModuleInterface
  {
    public:
      ServerModuleInterface(CharlieClient* cli);

      std::shared_ptr<Crypto> getSessionCrypto();
      std::string getClientId();
      std::string getClientPubkey();
      void send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target = NULL);
      void send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data);

    private:
      CharlieClient* client;
  };
}
