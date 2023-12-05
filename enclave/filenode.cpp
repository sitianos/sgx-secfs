#include "filenode.hpp"

#include <cstring>

Filenode::Filenode(const filenode_buffer_t& buf)
    : size(buf.size), chunks(buf.entry, buf.entry + buf.entnum) {
    ino = buf.ino;
}

size_t Filenode::dump(void* buf, size_t size) const {
    size_t rqsize = sizeof(filenode_buffer_t) + sizeof(chunk_t) * chunks.size();
    if (buf == nullptr) {
        return rqsize;
    }
    if (size < rqsize) {
        return 0;
    }
    filenode_buffer_t* obuf = static_cast<filenode_buffer_t*>(buf);
    obuf->ino = ino;
    obuf->size = this->size;
    obuf->entnum = chunks.size();
    for (size_t i = 0; i < chunks.size(); i++) {
        chunks[i].dump(obuf->entry[i]);
    }
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

Filenode::Chunk::Chunk(const chunk_t& chunk) : uuid(chunk.uuid), modified(false), mem(nullptr) {
}

Filenode::Chunk::Chunk(Filenode::Chunk&& chunk)
    : uuid(chunk.uuid), modified(chunk.modified), mem(chunk.mem) {
    chunk.mem = nullptr;
}

Filenode::Chunk::~Chunk() {
    free(mem);
}

void Filenode::Chunk::allocate() {
    if (mem == nullptr) {
        mem = new char[CHUNKSIZE];
    }
}

void Filenode::Chunk::deallocate() {
    if (mem) {
        delete[] mem;
        mem = nullptr;
    }
}

void Filenode::Chunk::dump(chunk_t& chunk) const {
    uuid.dump(chunk.uuid);
}
