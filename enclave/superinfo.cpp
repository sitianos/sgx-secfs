#include "superinfo.hpp"

#include <cstring>

bool Superinfo::load(const void* buf, size_t bsize) {
    size_t rqsize = sizeof(superinfo_buffer_t);
    if (bsize != rqsize) {
        return false;
    }
    const superinfo_buffer_t* obuf = static_cast<const superinfo_buffer_t*>(buf);
    root_dirnode.load(obuf->root_dirnode);
    user_table.load(obuf->user_table);
    std::memcpy(hash_root_dirnode, obuf->hash_root_dirnode, sizeof(hash_t));
    std::memcpy(hash_user_table, obuf->hash_user_table, sizeof(hash_t));
    max_ino = obuf->max_ino;
    return true;
}

size_t Superinfo::dump(void* buf, size_t bsize) const {
    size_t rqsize = sizeof(superinfo_buffer_t);
    if (buf == nullptr) {
        return rqsize;
    }
    if (bsize < rqsize) {
        return 0;
    }

    superinfo_buffer_t* obuf = static_cast<superinfo_buffer_t*>(buf);

    root_dirnode.dump(obuf->root_dirnode);
    user_table.dump(obuf->user_table);
    std::memcpy(obuf->hash_root_dirnode, hash_root_dirnode, sizeof(hash_t));
    std::memcpy(obuf->hash_user_table, hash_user_table, sizeof(hash_t));
    obuf->max_ino = max_ino;
    return rqsize;
}
