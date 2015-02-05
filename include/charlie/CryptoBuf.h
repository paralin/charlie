#pragma once

#include <charlie/Crypto.h>
#include <protogen/charlie.pb.h>

int decryptRsaBuf(charlie::CRSABuffer* buf, Crypto* crypt, unsigned char** outBuf, bool useRemote=true);
int verifySignedBuf(charlie::CSignedBuffer* buf, Crypto * crypt, bool useRemote=true);
int updateSignedBuf(charlie::CSignedBuffer* buf, Crypto * crypt);
