#pragma once
#include <cstdlib>
#include <string>

std::string gen_random(const int len) {
  char* s = (char *)malloc(sizeof(char) * (len + 1));
  static const char alphanum[] =
    "0123456789"
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
    "abcdefghijklmnopqrstuvwxyz";

  for (int i = 0; i < len; ++i) {
    s[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
  }

  s[len] = 0;
  std::string result(s);
  free(s);
  return result;
}
