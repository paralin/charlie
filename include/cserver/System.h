#pragma once

#include <Common.h>
#include <Logging.h>
#include <charlie/Crypto.h>
#include <charlie/base64.h>
#include <charlie/xor.h>
#include <protogen/charlie.pb.h>

class System {
  public:
    System();
    ~System();

    int main(int argc, const char* argv[]);

  private:
    Crypto * crypt;

    int loadCrypto();
};
