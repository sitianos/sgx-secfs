#include "dirnode.hpp"

#include <cstring>

Dirnode::Dirnode(const dirnode_buffer_t& buf)
    : name(buf.name), dirent(buf.entry, buf.entry + buf.entnum) {
    ino = buf.ino;
}

size_t Dirnode::dump(void* buf, size_t size) const {
    size_t rqsize = sizeof(dirnode_buffer_t) + sizeof(dirent_t) * dirent.size();
    if (buf == nullptr) {
        return rqsize;
    }
    if (size < rqsize) {
        return 0;
    }
    dirnode_buffer_t* obuf = static_cast<dirnode_buffer_t*>(buf);
    obuf->ino = ino;
    std::memset(obuf->name, 0, sizeof(obuf->name));
    name.copy(obuf->name, sizeof(obuf->name) - 1);
    obuf->entnum = dirent.size();
    for (size_t i = 0; i < dirent.size(); i++) {
        dirent[i].dump(obuf->entry[i]);
    }
    return rqsize;
}

void Dirnode::dump_stat(stat_buffer_t* buf) const {
    buf->ino = ino;
    buf->mode = T_ST_DIR;
    buf->nlink = nlink();
    buf->size = dump(nullptr, 0);
}

size_t Dirnode::nlink() const {
    size_t cnt = 2;
    for (auto& dent : dirent) {
        if (dent.type == T_DT_DIR)
            cnt++;
    }
    return cnt;
}

Dirnode::Dirent::Dirent(const dirent_t& dent) : ino(dent.ino), name(dent.name), type(dent.type), uuid(dent.uuid) {
}

void Dirnode::Dirent::dump(dirent_t& dent) const {
    dent.ino = ino;
    std::memset(dent.name, 0, sizeof(dent.name));
    name.copy(dent.name, sizeof(dent.name));
    dent.type = type;
    uuid.dump(dent.uuid);
}
