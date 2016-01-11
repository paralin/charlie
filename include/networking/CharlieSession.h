#pragma once
#ifndef CHARLIE_SESSION_H
#define CHARLIE_SESSION_H

#define CHARLIE_MAGIC ((char) 81)

#include <Common.h>
#include <IntTypes.h>

#include <protogen/charlie.pb.h>
#include <protogen/charlie_net.pb.h>

#include <charlie/Crypto.h>
#include <charlie/CryptoBuf.h>

#include <boost/asio.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/thread.hpp>

#include <networking/EMsgSizes.h>
#include <networking/ISessionController.h>

using boost::asio::ip::tcp;

class CharlieSession
{
  public:
    CharlieSession(std::shared_ptr<tcp::socket> socket, ISessionController* controller, std::string& priKey);
    ~CharlieSession();

    void start();

    void send(charlie::EMsg emsg, std::string& data, charlie::CMessageTarget* target = NULL);
    void send(u32 targetModule, u32 targetEmsg, u32 jobid, std::string& data);

    bool decryptBody();
    bool validateMessageHeader();
    bool validateMessageBody();
    void setController(ISessionController* cont);

    void disconnect();
    std::shared_ptr<Crypto> sessionCrypto;

    std::string remotePubkey;

  private:
    void doRead();
    void doKeepalive();

    void handleReadData();
    void handleHandshakeMessage();
    void handleIdentify();
    void handleAccept();

    void sendIdentify();
    void sendAccept();

    char headerLengthPrefixBuf[4];
    std::vector<char> buffer;

    bool expectingHeaderLengthPrefix;
    u32 expectedHeaderSize;
    bool expectingHeader;
    u32 expectedBodySize;
    bool handshakeComplete;

    charlie::CMessageHeader head;
    std::string decryptedData;

    std::time_t timeConnected;
    std::time_t lastMsg;

    bool receivedIdentify;

    std::string localChallenge;
    std::string remoteChallenge;

    boost::mutex sendMtx;
    bool hasDisconnected;

    std::shared_ptr<tcp::socket> socket;
    ISessionController* controller;

    boost::thread keepaliveThread;
#ifdef IS_CSERVER
    char incMagic;
#endif
};

#endif
