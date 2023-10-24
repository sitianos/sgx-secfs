#include "superinfo.hpp"

#include <cstring>

Superinfo Superinfo::load_from_buffer(const Superinfo::Buffer& buf) {
    Superinfo sp;
    sp.root_dirnode = UUID(buf.root_dirnode);
    sp.user_table = UUID(buf.user_table);
    std::memcpy(sp.hash_root_dirnode, buf.hash_root_dirnode, sizeof(hash_t));
    std::memcpy(sp.hash_user_table, buf.hash_user_table, sizeof(hash_t));
    return sp;
}

Superinfo::Buffer Superinfo::dump_to_buffer() {
    Superinfo::Buffer buf;
    root_dirnode.dump(buf.root_dirnode);
    user_table.dump(buf.user_table);
    std::memcpy(buf.hash_root_dirnode, hash_root_dirnode, sizeof(hash_t));
    std::memcpy(buf.hash_user_table, hash_user_table, sizeof(hash_t));
    return buf;
}
