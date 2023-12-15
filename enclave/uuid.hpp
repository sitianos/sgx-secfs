#pragma once

#include <functional>
#include <string>

class UUID {
  public:
    unsigned char data[16];

    UUID();
    UUID(const UUID& uuid);
    UUID(const unsigned char* bytes);
    UUID& operator=(const UUID& UUID);
    void dump(unsigned char* out) const;

    void unparse(char* out) const;
    void unparse(std::string& out) const;
    static UUID gen_rand();
};

template <>
struct std::hash<UUID> {
    size_t operator()(const UUID& uid) const;
};
