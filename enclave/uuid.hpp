#pragma once

#include <functional>
#include <string>

class UUID {
  public:
    UUID();
    UUID(const UUID& uuid);
    UUID(const uint8_t* bytes);

    UUID& operator=(const UUID& uuid);
    bool operator==(const UUID& uuid) const;
    void load(const uint8_t* in);
    void dump(uint8_t* out) const;
    void unparse(char* out) const;
    void unparse(std::string& out) const;

    uint8_t data[16];

    static UUID gen_rand();
};

template <>
struct std::hash<UUID> {
    size_t operator()(const UUID& uid) const;
};
