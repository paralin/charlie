#include <ctime>
#include <iostream>

#include <cserver/System.h>
#include <cserver/NetHost.h>
#include <cserver/ModuleTable.h>

#include <boost/filesystem.hpp>
#include <boost/algorithm/string/predicate.hpp>
#include <boost/thread/thread.hpp>

#include <boost/bind.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/asio.hpp>

NetHost::NetHost(System* sys)
{
  this->sys = sys;
  this->io_service = new boost::asio::io_service();
  tcp::endpoint endp(tcp::v4(), 9922);
  this->acceptor = new tcp::acceptor(*io_service, endp);
  do_accept();
}

void NetHost::do_accept()
{
  this->socket = std::shared_ptr<tcp::socket>(new tcp::socket(*io_service));
  acceptor->async_accept(*socket,
    [this](boost::system::error_code ec)
    {
    if (!ec)
    {
      CLOG("Accepted connection.");
      std::shared_ptr<CharlieClient> cli(new CharlieClient(socket, this));
      clients.push_back(cli);
      cli->start();
    } else
    {
      CERR("Error accepting connection, " << ec);
    }

    do_accept();
  });
}

NetHost::~NetHost()
{
  sys = NULL;
  delete acceptor;
  delete io_service;
}

void NetHost::mainThread()
{
  CLOG("Starting net host on port 9922...");
  while (true)
  {
    try {
      io_service->run();
    }
    catch (std::exception& e)
    {
      CERR("Internal io_service exception: " << e.what());
    }
  }
}

void NetHost::clientCreated(std::shared_ptr<CharlieClient> client)
{
  clients.push_back(client);
}

void NetHost::clientDisconnected(std::shared_ptr<CharlieClient> client)
{
  for(std::vector<std::shared_ptr<CharlieClient>>::iterator it = clients.begin(); it != clients.end(); ++it)
  {
    if (it->get() == client.get())
    {
      clients.erase(clients.begin(), it);
      break;
    }
  }
}
