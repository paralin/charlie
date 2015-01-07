#include <charlie/xor.h>

using namespace std;

int apply_xor(char* toEncrypt, int encryptLen, const char* key, int keyLen) {
  int keyi = 0;
  for (int i = 0; i < encryptLen; i++)
  {
    toEncrypt[i] = toEncrypt[i] ^ key[keyi];
    if(keyi++ >= keyLen) keyi = 0;
  }
  return encryptLen;
}

