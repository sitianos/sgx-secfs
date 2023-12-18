#pragma once

#include "metadata.hpp"

#include <vector>

class Filenode : public Inode {
  private:
  public:
    class Chunk {
      public:
        Chunk() = default;
        Chunk(const chunk_t* chunk);
        Chunk(const chunk_t& chunk);
        Chunk(Chunk&& chunk);
        ~Chunk();

        void allocate();
        void deallocate();
        void dump(chunk_t* chunk) const;

        UUID uuid;
        bool modified;
        char* mem;
    };

    Filenode() = default;
    Filenode(const filenode_buffer_t* buf);
    Filenode(const void* buf);
    ~Filenode() override = default;

    size_t dump(void* buf, size_t size) const override;
    void dump_stat(stat_buffer_t* buf) const override;
    size_t nlink() const override;

    size_t size;
    std::vector<Chunk> chunks;
};
