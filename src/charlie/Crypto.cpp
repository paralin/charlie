#include <charlie/Crypto.h>
#include <charlie/base64.h>
#include <openssl/evp.h>
#include <openssl/rsa.h>

Crypto::Crypto() {
  localKeypair  = NULL;
  remotePubKey  = NULL;

  init();
}

Crypto::Crypto(unsigned char *remotePubKey, size_t remotePubKeyLen) {
  this->localKeypair        = NULL;
  this->remotePubKey  = NULL;

  setRemotePubKey(remotePubKey, remotePubKeyLen);
  init();
}

Crypto::~Crypto() {
  if(remotePubKey)
    EVP_PKEY_free(remotePubKey);
  if(localKeypair)
    EVP_PKEY_free(localKeypair);

  EVP_CIPHER_CTX_cleanup(rsaEncryptCtx);
  EVP_CIPHER_CTX_cleanup(rsaDecryptCtx);

  free(rsaEncryptCtx);
  free(rsaDecryptCtx);
}

int Crypto::rsaEncrypt(const unsigned char *msg, size_t msgLen, unsigned char **encMsg, unsigned char **ek, int *ekl, unsigned char **iv, int *ivl, bool useRemote) {
  size_t encMsgLen = 0;
  size_t blockLen  = 0;

  EVP_PKEY *key;
  if(useRemote) key = remotePubKey;
  else          key = localKeypair;

  *ek = (unsigned char*)malloc(EVP_PKEY_size(key));
  *iv = (unsigned char*)malloc(EVP_MAX_IV_LENGTH);
  if(*ek == NULL || *iv == NULL) return FAILURE;
  *ivl = EVP_MAX_IV_LENGTH;

  *encMsg = (unsigned char*)malloc(msgLen + EVP_MAX_IV_LENGTH);
  if(encMsg == NULL) return FAILURE;

  if(!EVP_SealInit(rsaEncryptCtx, EVP_aes_256_cbc(), ek, (int*)ekl, *iv, &key, 1)) {
    return FAILURE;
  }

  if(!EVP_SealUpdate(rsaEncryptCtx, *encMsg + encMsgLen, (int*)&blockLen, (const unsigned char*)msg, (int)msgLen)) {
    return FAILURE;
  }
  encMsgLen += blockLen;

  if(!EVP_SealFinal(rsaEncryptCtx, *encMsg + encMsgLen, (int*)&blockLen)) {
    return FAILURE;
  }
  encMsgLen += blockLen;

  EVP_CIPHER_CTX_cleanup(rsaEncryptCtx);

  return (int)encMsgLen;
}

int Crypto::rsaDecrypt(unsigned char *encMsg, size_t encMsgLen, unsigned char *ek, size_t ekl, unsigned char *iv, size_t ivl, unsigned char **decMsg, bool useRemote) {
  size_t decLen   = 0;
  size_t blockLen = 0;
  EVP_PKEY *key;

  *decMsg = (unsigned char*)malloc(encMsgLen + ivl);
  if(decMsg == NULL) return FAILURE;

  if(useRemote) key = remotePubKey;
  else          key = localKeypair;

  if(!EVP_OpenInit(rsaDecryptCtx, EVP_aes_256_cbc(), ek, ekl, iv, key)) {
    return FAILURE;
  }

  if(!EVP_OpenUpdate(rsaDecryptCtx, (unsigned char*)*decMsg + decLen, (int*)&blockLen, encMsg, (int)encMsgLen)) {
    return FAILURE;
  }
  decLen += blockLen;

  if(!EVP_OpenFinal(rsaDecryptCtx, (unsigned char*)*decMsg + decLen, (int*)&blockLen)) {
    return FAILURE;
  }
  decLen += blockLen;

  EVP_CIPHER_CTX_cleanup(rsaDecryptCtx);

  return (int)decLen;
}

int Crypto::digestSign(const unsigned char*msg, size_t msgLen, unsigned char** sig, bool useRemote)
{
  size_t sigLen = EVP_MAX_MD_SIZE;

  EVP_PKEY *key;
  if(useRemote) key = remotePubKey;
  else          key = localKeypair;

  EVP_MD_CTX *mdctx;
  if((mdctx = EVP_MD_CTX_create()) == NULL)
    return FAILURE;

  if(1 != EVP_DigestSignInit(mdctx, NULL, EVP_sha256(), NULL, key))
    return FAILURE;

  if(1 != EVP_DigestSignUpdate(mdctx, msg, msgLen))
    return FAILURE;

  if(1 != EVP_DigestSignFinal(mdctx, NULL, &sigLen))
    return FAILURE;

  *sig = (unsigned char*)malloc(sigLen*sizeof(unsigned char));
  if(1 != EVP_DigestSignFinal(mdctx, *sig, &sigLen))
    return FAILURE;

  EVP_MD_CTX_destroy(mdctx);

  return (int)sigLen;
}

int Crypto::digestVerify(const unsigned char*msg, size_t msgLen, unsigned char* sig, size_t sigLen, bool useRemote)
{
  EVP_PKEY *key;
  if(useRemote) key = remotePubKey;
  else          key = localKeypair;

  EVP_MD_CTX *mdctx;
  if((mdctx = EVP_MD_CTX_create()) == NULL)
    return FAILURE;

  if(!EVP_DigestVerifyInit(mdctx, NULL, EVP_sha256(), NULL, key))
    return FAILURE;

  if(!EVP_DigestVerifyUpdate(mdctx, (const void*)msg, (unsigned int)msgLen))
    return FAILURE;

  if(!EVP_DigestVerifyFinal(mdctx, sig, sigLen))
    return FAILURE;

  EVP_MD_CTX_destroy(mdctx);

  return SUCCESS;
}

int Crypto::getRemotePubKey(unsigned char **pubKey) {
  BIO *bio = BIO_new(BIO_s_mem());
  PEM_write_bio_PUBKEY(bio, remotePubKey);

  int pubKeyLen = BIO_pending(bio);
  *pubKey = (unsigned char*)malloc(pubKeyLen);
  if(pubKey == NULL) return FAILURE;

  BIO_read(bio, *pubKey, pubKeyLen);

  // Insert the NUL terminator
  (*pubKey)[pubKeyLen-1] = '\0';

  BIO_free_all(bio);

  return pubKeyLen;
}

int Crypto::setRemotePubKey(unsigned char* pubKey, size_t pubKeyLen) {
  //BIO *bio = BIO_new(BIO_f_base64());
  BIO *bio = BIO_new(BIO_s_mem());
  if(BIO_write(bio, pubKey, pubKeyLen) != (int)pubKeyLen) {
    return FAILURE;
  }

  PEM_read_bio_PUBKEY(bio, &remotePubKey, NULL, NULL);

  BIO_free_all(bio);

  return SUCCESS;
}

int Crypto::getLocalPubKey(unsigned char** pubKey) {
  BIO *bio = BIO_new(BIO_s_mem());
  PEM_write_bio_PUBKEY(bio, localKeypair);

  int pubKeyLen = BIO_pending(bio);
  *pubKey = (unsigned char*)malloc(pubKeyLen);
  if(pubKey == NULL) return FAILURE;

  BIO_read(bio, *pubKey, pubKeyLen);

  // Insert the NUL terminator
  (*pubKey)[pubKeyLen-1] = '\0';

  BIO_free_all(bio);

  return pubKeyLen;
}

int Crypto::getLocalPriKey(unsigned char **priKey) {
  BIO *bio = BIO_new(BIO_s_mem());

  PEM_write_bio_PrivateKey(bio, localKeypair, NULL, NULL, 0, 0, NULL);

  int priKeyLen = BIO_pending(bio);
  *priKey = (unsigned char*)malloc(priKeyLen + 1);
  if(priKey == NULL) return FAILURE;

  BIO_read(bio, *priKey, priKeyLen);

  // Insert the NUL terminator
  (*priKey)[priKeyLen] = '\0';

  BIO_free_all(bio);

  return priKeyLen;
}

int Crypto::setLocalPriKey(unsigned char* priKey, size_t priKeyLen) {
  BIO *bio = BIO_new(BIO_s_mem());
  if(BIO_write(bio, priKey, priKeyLen) != (int)priKeyLen) {
    return FAILURE;
  }

  PEM_read_bio_PrivateKey(bio, &localKeypair, NULL, NULL);

  BIO_free_all(bio);

  return SUCCESS;
}

int Crypto::setLocalPubKey(unsigned char* pubKey, size_t pubKeyLen) {
  BIO *bio = BIO_new(BIO_s_mem());
  if(BIO_write(bio, pubKey, pubKeyLen) != (int)pubKeyLen) {
    return FAILURE;
  }

  PEM_read_bio_PUBKEY(bio, &localKeypair, NULL, NULL);

  BIO_free_all(bio);

  return SUCCESS;
}

int Crypto::init() {
  // Initalize contexts
  rsaEncryptCtx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));
  rsaDecryptCtx = (EVP_CIPHER_CTX*)malloc(sizeof(EVP_CIPHER_CTX));

  // Init these here to make valgrind happy
  EVP_CIPHER_CTX_init(rsaEncryptCtx);
  EVP_CIPHER_CTX_init(rsaDecryptCtx);

  return SUCCESS;
}

int Crypto::genLocalKeyPair() {
  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);

  if(EVP_PKEY_keygen_init(ctx) <= 0) {
    return FAILURE;
  }

  if(EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEYLEN) <= 0) {
    return FAILURE;
  }

  //Generate the local keypair (remove this?)
  if(EVP_PKEY_keygen(ctx, &localKeypair) <= 0) {
    return FAILURE;
  }

  EVP_PKEY_CTX_free(ctx);

  return SUCCESS;
}
