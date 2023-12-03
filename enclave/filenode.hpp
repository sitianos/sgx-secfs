#pragma once

#include "metadata.hpp"

#include <vector>

class Filenode : public Inode {
  private:
  public:
    class Chunk {
      public:
        UUID uuid;
        bool modified;
        unsigned char* mem;

        Chunk() = default;
        Chunk(const chunk_t& chunk);
        Chunk(const Chunk& chunk);
        Chunk(Chunk&& chunk);
        ~Chunk();

        void dump(chunk_t& chunk) const;
    };

    size_t size;
    std::vector<Chunk> chunks;
    unsigned long ctr[2];

    Filenode() = default;
    Filenode(const filenode_buffer_t &buf);

    size_t dump(void* buf, size_t size) const override;
    void dump_stat(stat_buffer_t* buf) const override;
    size_t nlink() const override;
};
