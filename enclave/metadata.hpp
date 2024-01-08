#pragma once

#include "internal.hpp"
#include "uuid.hpp"

#include <stdexcept>

class Metadata {
  public:
    Metadata(const UUID& _uuid);
    virtual ~Metadata() = default;

    virtual bool load(const void* buf, size_t bsize) = 0;
    // if buf == nullptr, returns required size
    // if size < required size, returns 0
    virtual size_t dump(void* buf, size_t bsize) const = 0;

    UUID uuid;
};

class Inode : public Metadata {
  public:
    // using Metadata::Metadata;
    Inode(const UUID& _uuid);

    virtual void dump_stat(stat_buffer_t* buf) const = 0;
    virtual size_t nlink() const = 0;

    ino_t ino;
    uint32_t refcount;
};
