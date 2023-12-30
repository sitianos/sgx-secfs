#pragma once

#include <functional>
#include <string>

class UUID {
  public:
    UUID();
    UUID(const UUID& uuid);
    UUID(const unsigned char* bytes);

    UUID& operator=(const UUID& uuid);
    bool operator==(const UUID& uuid);
    void load(const unsigned char* in);
    void dump(unsigned char* out) const;
    void unparse(char* out) const;
    void unparse(std::string& out) const;

    unsigned char data[16];

    static UUID gen_rand();
};

template <>
struct std::hash<UUID> {
    size_t operator()(const UUID& uid) const;
};
