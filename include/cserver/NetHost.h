#pragma once

#include <Common.h>
#include <Logging.h>

#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>
#include <protogen/manager.pb.h>
#include <protogen/directconnect.pb.h>

#include <cserver/networking/CharlieClient.h>
#include <boost/asio.hpp>

using boost::asio::ip::tcp;

class System;
class NetHost {
  public:
    NetHost(System* sys);
    ~NetHost();

    System* sys;

    void clientCreated(std::shared_ptr<CharlieClient> client);
    void clientDisconnected(std::shared_ptr<CharlieClient> client);

    void mainThread();
  private:
    boost::asio::io_service* io_service;
    tcp::acceptor* acceptor;

    std::shared_ptr<tcp::socket> socket;
    std::vector<std::shared_ptr<CharlieClient>> clients;

    void do_accept();
};
