#pragma once

#include <Common.h>
#include <IntTypes.h>
#include <protogen/charlie_net.pb.h>
#include <string>

// Verify a hashcash proof
bool verifyHashcashProof(std::string& stamp, std::string& tag, u32 difficulty, time_t* stamp_time);
std::string createHashcashProof(std::string& tag, u32 difficulty);
