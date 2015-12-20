#include <openssl/evp.h>
#include <openssl/pem.h>
#include <openssl/aes.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#define RSA_KEYLEN 2048
int main()
{
  // Key generation
  EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new_id(EVP_PKEY_RSA, NULL);
  EVP_PKEY* key = NULL;
  EVP_PKEY_keygen_init(ctx);
  EVP_PKEY_CTX_set_rsa_keygen_bits(ctx, RSA_KEYLEN);
  EVP_PKEY_keygen(ctx, &key);
  EVP_PKEY_CTX_free(ctx);

  // Serialize to string
  unsigned char* keyStr;
  BIO *bio = BIO_new(BIO_s_mem());
  PEM_write_bio_PrivateKey(bio, key, NULL, NULL, 0, 0, NULL);
  int priKeyLen = BIO_pending(bio);
  keyStr = (unsigned char*)malloc(priKeyLen + 1);
  BIO_read(bio, keyStr, priKeyLen);
  keyStr[priKeyLen] = '\0';
  BIO_free_all(bio);

  // Print the string
  printf("%s", keyStr);

  // Reset the key
  EVP_PKEY_free(key);
  key = NULL;

  // Read from string
  bio = BIO_new(BIO_s_mem());
  BIO_write(bio, keyStr, priKeyLen);
  PEM_read_bio_PrivateKey(bio, &key, NULL, NULL);
  BIO_free_all(bio);

  // Free the string
  free(keyStr);

  // Serialize to string (again)
  bio = BIO_new(BIO_s_mem());
  PEM_write_bio_PrivateKey(bio, key, NULL, NULL, 0, 0, NULL);
  priKeyLen = BIO_pending(bio);
  keyStr = (unsigned char*)malloc(priKeyLen + 1);
  BIO_read(bio, keyStr, priKeyLen); 
  keyStr[priKeyLen] = '\0';
  BIO_free_all(bio);

  // Print string
  printf("%s", keyStr);
  
  // Free the string
  free(keyStr);

  // Serialize to string (again)
  bio = BIO_new(BIO_s_mem());
  PEM_write_bio_PUBKEY(bio, key);
  priKeyLen = BIO_pending(bio);
  keyStr = (unsigned char*)malloc(priKeyLen + 1);
  BIO_read(bio, keyStr, priKeyLen); 
  keyStr[priKeyLen] = '\0';
  BIO_free_all(bio);

  // Print string
  printf("%s", keyStr);
}
