#pragma once
#include <string>
#include <set>
#include <IntTypes.h>
#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>
#include <charlie/Crypto.h>

namespace server_modules
{
  // Server module interface
  class SModuleInterface
  {
    public:
      ~SModuleInterface() {}
      //Return the crypto class
      virtual std::shared_ptr<Crypto> getSessionCrypto() = 0;
      virtual std::string getClientId() = 0;
      virtual std::string getClientPubkey() = 0;
      virtual void send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target = NULL) = 0;
      virtual void send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data) = 0;
      virtual void disconnect() = 0;
  };
};
