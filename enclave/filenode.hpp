#pragma once

#include "metadata.hpp"

#include <vector>

class Chunk {
  public:
    Chunk() = default;
    Chunk(const chunk_t* chunk);
    Chunk(const chunk_t& chunk);
    Chunk(const Chunk& chunk) = delete;
    Chunk(Chunk&& chunk);
    ~Chunk();

    Chunk& operator=(const Chunk& chunk) = delete;
    void dump(chunk_t* chunk) const;

    UUID uuid;
    iv_t iv;
    tag_t tag;
};

class ChunkTable {
  public:
    UUID uuid;
    std::vector<Chunk> entry;
};

class Filenode : public Inode {
  private:
  public:
    using Inode::Inode;
    ~Filenode() override = default;

    bool load(const void* buf, size_t bsize) override;
    size_t dump(void* buf, size_t bsize) const override;
    void dump_stat(stat_buffer_t* buf) const override;
    size_t nlink() const override;

    size_t size;
    std::vector<Chunk> chunks;
};
