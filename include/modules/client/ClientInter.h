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
      void send(charlie::EMsg emsg, u32 target, std::string& data, u32 jobid = 0, u32 targetEmsg = 0);
      void send(u32 targetModule, u32 jobId, u32 targetEmsg, std::string& data);

    private:
      ClientModule* mod;
    };
  }
}
