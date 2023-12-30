#pragma once

#include "internal.hpp"
#include "uuid.hpp"

#include <stdexcept>

class Metadata {
  public:
    // Metadata(const UUID& uuid);
    virtual ~Metadata() = default;

    virtual bool load(const void* buf, size_t bsize) = 0;
    // if buf == nullptr, returns required size
    // if size < required size, returns 0
    virtual size_t dump(void* buf, size_t bsize) const = 0;

    UUID uuid;

    template <typename T>
    static std::enable_if_t<std::is_base_of<Metadata, T>::value, T*>
    create(const UUID& uuid, const void* buf = nullptr, size_t size = 0);
};

class Inode : public Metadata {
  public:
    virtual void dump_stat(stat_buffer_t* buf) const = 0;
    virtual size_t nlink() const = 0;

    ino_t ino;
};

template <typename T>
std::enable_if_t<std::is_base_of<Metadata, T>::value, T*>
Metadata::create(const UUID& uuid, const void* buf, size_t bsize) {
    (void)bsize;
    T* ret = nullptr;
    if (buf) {
        ret = new T(buf);
    } else {
        ret = new T;
    }
    if (ret) {
        ret->uuid = uuid;
    }
    return ret;
}
