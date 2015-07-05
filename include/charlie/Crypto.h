#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <stdio.h>
#include <string>
#include <string.h>

#include <boost/thread/mutex.hpp>

#ifndef CRYPTO_H
#define CRYPTO_H

#define RSA_KEYLEN 2048

#define SUCCESS 0
#define FAILURE -1

#define KEY_SERVER_PRI 0
#define KEY_SERVER_PUB 1
#define KEY_CLIENT_PUB 2

class Crypto {

public:
    Crypto();

    Crypto(unsigned char *remotePubKey, size_t remotePubKeyLen);

    ~Crypto();

    int rsaEncrypt(const unsigned char *msg, size_t msgLen, unsigned char **encMsg, unsigned char **ek, int *ekl, unsigned char **iv, int *ivl, bool useRemote=true);

    int rsaDecrypt(unsigned char *encMsg, size_t encMsgLen, unsigned char *ek, size_t ekl, unsigned char *iv, size_t ivl, unsigned char **decMsg, bool useRemote=false);

    int digestSign(const unsigned char *msg, size_t msgLen, unsigned char** sig, bool useRemote=false);

    int digestVerify(const unsigned char *msg, size_t msgLen, unsigned char* sig, size_t sigLen, bool useRemote=false);

    int getRemotePubKey(unsigned char **pubKey);

    int setRemotePubKey(unsigned char *pubKey, size_t pubKeyLen);

    int setLocalPubKey(unsigned char *pubKey, size_t pubKeyLen);

    int setLocalPriKey(unsigned char *priKey, size_t priKeyLen);

    int getLocalPubKey(unsigned char **pubKey);

    int getLocalPriKey(unsigned char **priKey);

    int setLocalPubKey(unsigned char **pubKey, size_t pubKeyLen);

    int setLocalPriKey(unsigned char **priKey, size_t priKeyLen);

    int genLocalKeyPair();

private:
    EVP_PKEY *localKeypair;
    EVP_PKEY *remotePubKey;

    EVP_CIPHER_CTX *rsaEncryptCtx;
    EVP_CIPHER_CTX *rsaDecryptCtx;

    int init();
    boost::mutex cmtx;
};

#endif
