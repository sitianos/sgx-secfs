#pragma once

#include "uuid.hpp"

using uuid_t = unsigned char[16];
using hash_t = unsigned char[32];

class Metadata {
  private:
    UUID uuid;

  public:
    // virtual int load() = 0;
    // virtual int dump() = 0;
};
