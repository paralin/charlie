#pragma once
#include <Common.h>
#include <string>
#include <boost/asio.hpp>
#include <ModuleAPI.h>

using boost::asio::ip::tcp;

class CharlieSession;
namespace modules
{
  class VISIBLE ModuleNet : public ModuleAPI
  {
    public:
      virtual ~ModuleNet() {};

      // Imported from ModuleAPI
      virtual void handleCommand(const charlie::CMessageTarget& target, std::string& buf) = 0;
      // Is this network module able to communicate?
      virtual bool ready() = 0;
      // Disconnect, your connection is invalid.
      virtual void disconnectInvalid() = 0;
      // Return the socket.
      virtual std::shared_ptr<CharlieSession> getSession() = 0;
  };
}
