#include <Logging.h>
#include <charlie/CryptoBuf.h>

int decryptRsaBuf(charlie::CRSABuffer* buf, Crypto* crypt, unsigned char**outBuf, bool useRemote)
{
  return crypt->rsaDecrypt((unsigned char*)buf->data().c_str(), buf->data().length(), (unsigned char*)buf->ek().c_str(), buf->ek().length(), (unsigned char*)buf->iv().c_str(), buf->iv().length(), outBuf, useRemote);
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
