#include <stdlib.h>
#include <charlie/hash.h>
#include <iostream>
#include <fstream>
#include <string.h>
#include <openssl/sha.h>

unsigned int hashString(const char *str)
{
  unsigned int h;
  unsigned char *p;

  h = 0;
  for (p = (unsigned char*)str; *p != '\0'; p++)
    h = 37 * h + *p;
  return h;
}

int sha256File(const char* path, unsigned char** digest, char** fileData)
{
  bool saveData = fileData != NULL;
  //open the file
  std::ifstream inFile (path, std::ios::in|std::ios::binary|std::ios::ate);
  if(inFile.is_open())
  {
    size_t size = inFile.tellg();
    inFile.seekg (0, std::ios::beg);
    *digest = (unsigned char*)malloc(sizeof(unsigned char)*SHA256_DIGEST_LENGTH);
    if(saveData) *fileData = (char*)malloc(sizeof(char)*size);
    SHA256_CTX ctx;
    SHA256_Init(&ctx);
    //load file in 1000 byte blocks
    size_t si;
    char* memblock = (char*)malloc(sizeof(char)*1000);
    while(size > 0){
      size_t blsize = std::min(size,(size_t)1000);
      size -= blsize;
      inFile.read (memblock, blsize);
      SHA256_Update(&ctx, memblock, blsize);
      if(saveData)
        memcpy(fileData[si], memblock, blsize);
      si += blsize-1;
    }
    inFile.close();
    free(memblock);
    SHA256_Final(*digest, &ctx);
    return 0;
  }else return -1;
}
