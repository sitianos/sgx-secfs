#include "uuid.hpp"

#include "sgx_trts.h"

#include <cstdio>
#include <cstring>

UUID::UUID(bool rand) {
    if (rand)
        sgx_read_rand(data, sizeof(data));
    else
        std::memset(data, 0, sizeof(data));
}

UUID::UUID(const UUID& uuid) : UUID(uuid.data) {
}

UUID::UUID(const unsigned char* bytes) {
    std::memcpy(data, bytes, sizeof(data));
}

UUID& UUID::operator=(const UUID& uuid) {
    std::memcpy(data, uuid.data, sizeof(uuid.data));
    return *this;
}

void UUID::dump(unsigned char* out) const {
    std::memcpy(out, data, sizeof(data));
}

bool UUID::parse(const char* in) {
    // unsigned char tmp[16];
    // if (std::sscanf(in, "%02x%02x%02x%02x-%02x%02x-%02x%02x-%02x%02x-%02x%02x%02x%02x%02x%02x",
    //                 &tmp[0], &tmp[1], &tmp[2], &tmp[3], &tmp[4], &tmp[5], &tmp[6], &tmp[7],
    //                 &tmp[8], &tmp[9], &tmp[10], &tmp[11], &tmp[12], &tmp[13], &tmp[14], &tmp[15])
    //                 != 16) {
    //     return false;
    // }
    // std::memcpy(data, tmp, sizeof(data));
    return true;
}

bool UUID::parse(const std::string& in) {
    return parse(in.c_str());
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

size_t std::hash<UUID>::operator()(const UUID& uid) const {
    return *((size_t*)uid.data) + *((size_t*)&uid.data[8]);
}
