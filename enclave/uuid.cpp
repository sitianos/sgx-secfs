#include "uuid.hpp"

#include "sgx_trts.h"

#include <cstdio>
#include <cstring>

UUID::UUID() {
    std::memset(data, 0, sizeof(data));
}

UUID::UUID(const UUID& uuid) : UUID(uuid.data) {
}

UUID::UUID(const unsigned char* bytes) {
    std::memcpy(data, bytes, sizeof(data));
}

UUID& UUID::operator=(const UUID& uuid) {
    std::memcpy(data, uuid.data, sizeof(data));
    return *this;
}

bool UUID::operator==(const UUID& uuid) {
    return std::memcpy(data, uuid.data, sizeof(data)) == 0;
}

void UUID::dump(unsigned char* out) const {
    std::memcpy(out, data, sizeof(data));
}

void UUID::unparse(char* out) const {
    std::snprintf(out, 37, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
                  data[0], data[1], data[2], data[3], data[4], data[5], data[6], data[7], data[8],
                  data[9], data[10], data[11], data[12], data[13], data[14], data[15]);
}

void UUID::unparse(std::string& out) const {
    char str[37];
    unparse(str);
    out = str;
}

UUID UUID::gen_rand() {
    unsigned char data[16];
    sgx_read_rand(data, sizeof(data));
    return UUID(data);
}

size_t std::hash<UUID>::operator()(const UUID& uid) const {
    return *((size_t*)uid.data) + *((size_t*)&uid.data[8]);
}
