#pragma once

#include <Common.h>
#include <IntTypes.h>
#include <SModuleInterface.h>

#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>

#include <charlie/Crypto.h>
#include <charlie/CryptoBuf.h>

#include <cserver/ServerModuleInstance.h>

#include <boost/asio.hpp>
#include <boost/thread.hpp>
#include <boost/enable_shared_from_this.hpp>

#include <networking/EMsgSizes.h>
#include <networking/CharlieSession.h>

using boost::asio::ip::tcp;

class NetHost;
class CharlieClient : public std::enable_shared_from_this<CharlieClient>, public server_modules::SModuleInterface, public ISessionController
{
  public:
    CharlieClient(std::shared_ptr<tcp::socket>& socket, NetHost* host);
    ~CharlieClient();

    void start();
    void deliver(u32 target, charlie::EMsg emsg, const std::string& data);

    void send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target = NULL);
    void send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data);
    void disconnect();

    void handleMessage(charlie::CMessageHeader& head, std::string& data);
    void handleRoutedMessage(charlie::CMessageHeader& head, std::string& data);
    void handleFailure(std::string& data);

    void sendInitData();
    void sendModuleTable();
    void sendDeliveryFailure();

    void onDisconnected();
    void onHandshakeComplete();

    void calcClientId();

    // Methods for the modules to call
    inline std::shared_ptr<Crypto> getSessionCrypto() { return session->sessionCrypto; }
    inline std::string getClientId() { return clientId; }
    inline std::string getClientPubkey() { return clientPubkey; }

    std::vector<std::shared_ptr<ServerModuleInstance>> modules;

    NetHost* host;
    std::shared_ptr<CharlieSession> session;

  private:
    std::shared_ptr<tcp::socket> socket;
    std::vector<char> buffer;

    charlie::CMessageHeader head;

    std::string clientId;
    std::string clientPubkey;
};

typedef std::shared_ptr<CharlieClient> client_ptr;
