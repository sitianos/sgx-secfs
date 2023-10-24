#include "dirnode.hpp"

#include "cstdlib"

Dirnode Dirnode::load_from_buffer(const Dirnode::Buffer& buf) {
    Dirnode dn;
    dn.name = buf.name;
    dn.dirent.resize(buf.entnum);
    for (size_t i = 0; i < buf.entnum; i++) {
        dn.dirent[i] = Dirnode::Dirent(buf.entry[i]);
    }
    return dn;
}

size_t Dirnode::dump_to_buffer(Dirnode::Buffer*& buf) {
    size_t size = sizeof(Dirnode::Buffer) + sizeof(Dirnode::dirent_t) * dirent.size();
    buf = (Dirnode::Buffer*)malloc(size);
    name.copy(buf->name, sizeof(buf->name) - 1);
    buf->entnum = dirent.size();
    for (size_t i = 0; i < dirent.size(); i++) {
        dirent[i].dump(buf->entry[i]);
    }
    return size;
}

Dirnode::Dirent::Dirent() {
}

Dirnode::Dirent::Dirent(const Dirnode::dirent_t& dent)
    : name(dent.name), type(dent.type), uuid(dent.uuid) {
}

void Dirnode::Dirent::dump(Dirnode::dirent_t& dent) {
    name.copy(dent.name, MAX_PATH_LEN);
    dent.type = type;
    uuid.dump(dent.uuid);
}
