#pragma once

#include "metadata.hpp"

class Filenode : public Inode {
  private:
  public:
    Filenode() = default;
    Filenode(const filenode_buffer_t &buf);
    ino_t ino;
    size_t size;
    size_t dump(void* buf, size_t size) const override;
    void dump_stat(stat_buffer_t* buf) const override;
    size_t nlink() const override;
};
