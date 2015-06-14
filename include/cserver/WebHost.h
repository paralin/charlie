#pragma once

#include <Common.h>
#include <Logging.h>
#include <charlie/Crypto.h>
#include <charlie/base64.h>
#include <charlie/xor.h>
#include <protogen/charlie.pb.h>
#include <boost/network/protocol/http/server.hpp>

struct server_def;
typedef boost::network::http::server<server_def> server;

class System;
class WebHost {
  public:
    WebHost(System * sys);
    ~WebHost();

    void mainThread();

    void processRequest(server::request const &request, server::response &response);

  private:
    std::string info;
    System* sys;
};
