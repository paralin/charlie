#pragma once

#include <Common.h>
#include <Logging.h>
#include <charlie/Crypto.h>
#include <charlie/base64.h>
#include <charlie/xor.h>
#include <cserver/WebHost.h>
#include <cserver/NetHost.h>
#include <protogen/charlie.pb.h>
#include <cserver/ServerModuleInstance.h>

class CharlieClient;
class System {
  public:
    System();
    ~System();

    int main(int argc, const char* argv[]);

    Crypto * crypt;

    charlie::CModuleTable* generateModuleTableFromFile(const char* path = "./init.json");
    std::vector<std::shared_ptr<ServerModuleInstance>> buildModuleSet(CharlieClient* client);

    WebHost* host;
    NetHost* nHost;
    std::string privateKey;

  private:
    int loadCrypto();
    void webTh();
};
