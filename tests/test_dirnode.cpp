#include "../enclave/dirnode.hpp"

#include <cstdlib>
#include <cstring>
#include <iostream>
#include <string>

int main() {
    Dirnode::Buffer* buf =
        (Dirnode::Buffer*)malloc(sizeof(Dirnode::Buffer) + sizeof(Dirnode::dirent_t) * 2);
    *buf = {
        .name = "dir1",
        .entnum = 2,
    };
    buf->entry[0] = {.name = "file1",
                     .type = Dirnode::T_REG,
                     .uuid = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15}};
    buf->entry[1] = {.name = "dir2",
                     .type = Dirnode::T_DIR,
                     .uuid = {16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31}};
    Dirnode dn = Dirnode::load_from_buffer(*buf);
    std::cout << dn.name << '\n';
    for (auto& ent : dn.dirent) {
        std::string uu;
        ent.uuid.unparse(uu);
        std::cout << uu << " : " << ent.type << " " << ent.name << '\n';
    }
    free(buf);
}
