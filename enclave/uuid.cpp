#include "uuid.hpp"

#include <cstdio>
#include <cstring>

UUID::UUID() {
}

UUID::UUID(const UUID& uuid) : UUID(uuid.data) {
}

UUID::UUID(const unsigned char* bytes) {
    memcpy(data, bytes, sizeof(data));
}

UUID& UUID::operator=(const UUID& uuid) {
    memcpy(data, uuid.data, sizeof(uuid.data));
    return *this;
}

void UUID::dump(unsigned char* out) {
    memcpy(out, data, sizeof(data));
}

void UUID::unparse(char* out) {
    size_t i = 0;
    for (; i < sizeof(data); i++) {
        snprintf(&out[i * 2], 3, "%02x", data[i]);
    }
    out[i * 2] = '\0';
}

void UUID::unparse(std::string& out) {
    char str[sizeof(data) * 2 + 1];
    unparse(str);
    out = str;
}
