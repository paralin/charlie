#include <pow/pow.h>
#include <pow/hashcash.h>
#include <time.h>

bool verifyHashcashProof(std::string& stamp, std::string& tag, u32 difficulty, time_t* stamp_time)
{
  time_t now;
  time(&now);

  time_t st;
  int res = hashcash_check(stamp.c_str(), 0, tag.c_str(), NULL, NULL, TYPE_STR, now, 0, 500, difficulty, &st);
  if (stamp_time)
    *stamp_time = st;
  return res;
}

std::string createHashcashProof(std::string& tag, u32 difficulty)
{
  char* outp = hashcash_simple_mint(tag.c_str(), difficulty, 0, NULL, 1);
  return std::string(outp);
}
