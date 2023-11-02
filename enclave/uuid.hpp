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

    bool parse(const char* in);
    bool parse(const std::string& in);
    void unparse(char* out) const;
    void unparse(std::string& out) const;
};

template <> struct std::hash<UUID> {
    size_t operator()(const UUID& uid) const;
};
