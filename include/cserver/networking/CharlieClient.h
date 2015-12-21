#pragma once

#include <Common.h>
#include <IntTypes.h>

#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>

#include <charlie/Crypto.h>
#include <charlie/CryptoBuf.h>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <networking/EMsgSizes.h>

using boost::asio::ip::tcp;

class NetHost;
class CharlieClient : public std::enable_shared_from_this<CharlieClient>
{
  public:
    CharlieClient(std::shared_ptr<tcp::socket>& socket, NetHost* host);
    ~CharlieClient();

    void start();
    void deliver(u32 target, charlie::EMsg emsg, const std::string& data);
    void doRead();
    void handleReadData();

    void sendServerIdentify();
    void sendServerAccept();

    // Don't use this
    [[deprecated("Replaced by the other send.")]]
    void send(charlie::EMsg emsg, u32 targetModule, std::string& data, u32 jobid = 0, u32 targetEmsg = 0);
    void send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target = NULL);
    void send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data);

    bool validateMessageHeader();
    bool validateMessageBody();
    void handleMessage(std::string& data);

    bool handleClientIdentify(std::string& data);
    void handleClientAccept(std::string& data);
    void handleFailure(std::string& data);

    void disconnect();

    void sendInitData();
    void sendModuleTable();

    Crypto* sessionCrypto;
    Crypto* serverCrypto;

  private:
    NetHost* host;
    std::shared_ptr<tcp::socket> socket;
    std::vector<char> buffer;

    bool expectingHeaderLengthPrefix;
    u32 expectedHeaderSize;
    bool expectingHeader;
    u32 expectedBodySize;
    bool handshakeComplete;
    charlie::CMessageHeader head;
    charlie::CMessageBody body;
    std::time_t timeConnected;

    // Handshake state
    bool receivedClientIdentify;
    std::string clientChallenge;
    std::string serverChallenge;
};

typedef std::shared_ptr<CharlieClient> client_ptr;
