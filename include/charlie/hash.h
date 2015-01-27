#pragma once
#include <string>

unsigned int hashString(const char *str);
int sha256File(const char* path, unsigned char** digest, char** fileData=NULL);
