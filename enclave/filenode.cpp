#include "filenode.hpp"

Filenode::Filenode(const filenode_buffer_t &buf): size(buf.size){
    ino = buf.ino;
}

size_t Filenode::dump(void* buf, size_t size) const {
    size_t rqsize = sizeof(filenode_buffer_t);
    if (buf == nullptr) {
        return rqsize;
    }
    if (size < rqsize) {
        return 0;
    }
    filenode_buffer_t* obuf = static_cast<filenode_buffer_t*>(buf);
    obuf->ino = ino;
    obuf->size = size;
    return rqsize;
}

void Filenode::dump_stat(stat_buffer_t* buf) const {
    buf->ino = ino;
    buf->mode = T_ST_REG;
    buf->nlink = nlink();
    buf->size = size;
}

size_t Filenode::nlink() const {
    return 1;
}
