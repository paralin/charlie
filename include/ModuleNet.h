#pragma once
#include <Common.h>
#include <string>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

namespace modules
{
  class VISIBLE ModuleNet
  {
    public:
      virtual ~ModuleNet() {};

      // Is this network module able to communicate?
      virtual bool ready() = 0;
      // Disconnect, your connection is invalid.
      virtual void disconnectInvalid();
      // Return the socket.
      virtual tcp::socket* getSocket();
  };
}
