#include "../enclave/uuid.hpp"
#include <string>
#include <iostream>

int main() {
    const unsigned char bytes[16] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    UUID uuid(bytes);
    UUID uuid2(uuid);

    char ctr[33];
    uuid.unparse(ctr);
    std::cout << ctr << std::endl;

    std::string str;
    uuid2.unparse(str);
    std::cout << str << std::endl;
}
