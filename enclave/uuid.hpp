#include <string>

class UUID {
   public:
    unsigned char data[16];

    UUID();
    UUID(const UUID& uuid);
    UUID(const unsigned char *bytes);
    UUID& operator=(const UUID& UUID);
    void dump(unsigned char *out);
    void unparse(char *out);
    void unparse(std::string &out);

    // static UUID parse(char *in);
    // static UUID parse(std::string &in);
};

// class Hash {
//    public:
//     unsigned char data[32];
// };
