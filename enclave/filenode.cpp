#include "filenode.hpp"

#include <cstring>

bool Filenode::load(const void* buf, size_t bsize) {
    const filenode_buffer_t* obuf = static_cast<const filenode_buffer_t*>(buf);
    size_t rqsize = sizeof(filenode_buffer_t) + sizeof(chunk_t) * obuf->entnum;
    if (bsize != rqsize) {
        return false;
    }
    if ((obuf->size + CHUNKSIZE - 1) / CHUNKSIZE != obuf->entnum) {
        return false;
    }
    size = obuf->size;
    chunks = decltype(chunks)(obuf->entry, obuf->entry + obuf->entnum);
    ino = obuf->ino;
    return true;
}

size_t Filenode::dump(void* buf, size_t bsize) const {
    size_t rqsize = sizeof(filenode_buffer_t) + sizeof(chunk_t) * chunks.size();
    if (buf == nullptr) {
        return rqsize;
    }
    if (bsize < rqsize) {
        return 0;
    }
    filenode_buffer_t* obuf = static_cast<filenode_buffer_t*>(buf);
    obuf->ino = ino;
    obuf->size = this->size;
    obuf->entnum = chunks.size();
    for (size_t i = 0; i < chunks.size(); i++) {
        chunks[i].dump(&obuf->entry[i]);
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

Filenode::Chunk::Chunk(const chunk_t* chunk) : uuid(chunk->uuid), modified(false), mem(nullptr) {
    memcpy(iv, chunk->iv, sizeof(iv_t));
    memcpy(tag, chunk->tag, sizeof(tag_t));
}

Filenode::Chunk::Chunk(const chunk_t& chunk) : Chunk(&chunk) {
}

Filenode::Chunk::Chunk(Filenode::Chunk&& chunk)
    : uuid(chunk.uuid), modified(chunk.modified), mem(chunk.mem) {
    chunk.mem = nullptr;
    memcpy(iv, chunk.iv, sizeof(iv_t));
    memcpy(tag, chunk.tag, sizeof(tag_t));
}

Filenode::Chunk::~Chunk() {
    if (mem)
        delete[] mem;
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

void Filenode::Chunk::dump(chunk_t* chunk) const {
    uuid.dump(chunk->uuid);
    memcpy(chunk->iv, iv, sizeof(iv_t));
    memcpy(chunk->tag, tag, sizeof(tag_t));
}
