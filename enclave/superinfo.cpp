#include "superinfo.hpp"

#include <cstring>

Superinfo::Superinfo(const superinfo_buffer_t* buf)
    : root_dirnode(buf->root_dirnode), user_table(buf->user_table) {
    std::memcpy(hash_root_dirnode, buf->hash_root_dirnode, sizeof(hash_t));
    std::memcpy(hash_user_table, buf->hash_user_table, sizeof(hash_t));
    max_ino = buf->max_ino;
}

Superinfo::Superinfo(const void* buf) : Superinfo(static_cast<const superinfo_buffer_t*>(buf)) {
}

size_t Superinfo::dump(void* buf, size_t size) const {
    size_t rqsize = sizeof(superinfo_buffer_t);
    if (buf == nullptr) {
        return rqsize;
    }
    if (size < rqsize) {
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
