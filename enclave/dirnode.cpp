#include "dirnode.hpp"

#include <cstring>

bool Dirnode::load(const void* buf, size_t bsize) {
    const dirnode_buffer_t* obuf = static_cast<const dirnode_buffer_t*>(buf);
    size_t rqsize = sizeof(dirnode_buffer_t) + sizeof(dirent_t) * obuf->entnum;
    if (bsize != rqsize) {
        return false;
    }
    name = obuf->name;
    dirent.assign(obuf->entry, obuf->entry + obuf->entnum);
    ino = obuf->ino;
    return true;
}

size_t Dirnode::dump(void* buf, size_t bsize) const {
    size_t rqsize = sizeof(dirnode_buffer_t) + sizeof(dirent_t) * dirent.size();
    if (buf == nullptr) {
        return rqsize;
    }
    if (bsize < rqsize) {
        return 0;
    }
    dirnode_buffer_t* obuf = static_cast<dirnode_buffer_t*>(buf);
    obuf->ino = ino;
    std::memset(obuf->name, 0, sizeof(obuf->name));
    name.copy(obuf->name, sizeof(obuf->name) - 1);
    obuf->entnum = dirent.size();
    for (size_t i = 0; i < dirent.size(); i++) {
        dirent[i].dump(&obuf->entry[i]);
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

Dirent::Dirent(const dirent_t* dent)
    : ino(dent->ino), name(dent->name), type(dent->type), uuid(dent->uuid) {
}

Dirent::Dirent(const dirent_t& dent) : Dirent(&dent) {
}

void Dirent::dump(dirent_t* dent) const {
    dent->ino = ino;
    std::memset(dent->name, 0, sizeof(dent->name));
    name.copy(dent->name, sizeof(dent->name));
    dent->type = type;
    uuid.dump(dent->uuid);
}
