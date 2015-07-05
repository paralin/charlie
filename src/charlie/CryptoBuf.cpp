#include <Logging.h>
#include <charlie/CryptoBuf.h>

int decryptRsaBuf(charlie::CRSABuffer* buf, Crypto* crypt, unsigned char**outBuf, bool useRemote)
{
  return crypt->rsaDecrypt((unsigned char*)buf->data().c_str(), buf->data().length(), (unsigned char*)buf->ek().c_str(), buf->ek().length(), (unsigned char*)buf->iv().c_str(), buf->iv().length(), outBuf, useRemote);
}

int encryptRsaBuf(charlie::CRSABuffer* buf, Crypto* crypt, const unsigned char*data, size_t length, bool useRemote)
{
  unsigned char* encMsg = NULL;
  unsigned char* ek = NULL;
  int ekl;
  unsigned char* iv = NULL;
  int ivl;

  int res = crypt->rsaEncrypt(data, length, &encMsg, &ek, &ekl, &iv, &ivl);
  if(res == FAILURE) return FAILURE;

  buf->set_data(encMsg, res);
  buf->set_ek(ek, ekl);
  buf->set_iv(iv, ivl);

  free(encMsg);
  free(ek);
  free(iv);

  return SUCCESS;
}

int verifySignedBuf(charlie::CSignedBuffer* buf, Crypto * crypt, bool useRemote)
{
  return crypt->digestVerify((const unsigned char*)buf->data().c_str(), buf->data().length(), (unsigned char*)buf->sig().c_str(), buf->sig().length(), useRemote);
}

int updateSignedBuf(charlie::CSignedBuffer* buf, Crypto* crypt)
{
  std::string data = buf->data();
  unsigned char* sig;
  int sigLen = crypt->digestSign((const unsigned char*)data.c_str(), data.length(), &sig, false);
  if(sigLen == FAILURE) return FAILURE;
  buf->set_sig(sig, sigLen);
  free(sig);
  return SUCCESS;
}
