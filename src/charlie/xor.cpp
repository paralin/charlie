#include <Logging.h>
#include <charlie/xor.h>
#include <openssl/md5.h>

using namespace std;

int apply_xor(char* toEncrypt, int encryptLen, const char* key, int keyLen) {
  bool inc;
  int keyi = keyLen-1;
  for (int i = 0; i < encryptLen; i++)
  {
    toEncrypt[i] = toEncrypt[i] ^ key[keyi];
    if(keyi>=keyLen-1) { inc = false; }
    else if(keyi == 0) { inc = true;  }
    if(inc) keyi++;
    else    keyi--;
  }
  return encryptLen;
}

