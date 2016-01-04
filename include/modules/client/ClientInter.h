#pragma once
#include <Module.h>
#include <ModuleAPI.h>
#include <protogen/charlie_net.pb.h>

//This is the public interface
namespace modules
{
  namespace client
  {
    class ClientModule;
    class VISIBLE ClientInter : public ModuleAPI
    {
    public:
      ClientInter(ClientModule * mod);
      ~ClientInter();

      void handleCommand(const charlie::CMessageTarget& targ, std::string& buf);

      void send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target = NULL);
      void send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data);
      void retryConnectionsNow();

    private:
      ClientModule* mod;
    };
  }
}
