#pragma once

#include "bridge.hpp"
#include "uuid.hpp"

class Metadata {
  public:
    enum Type {
        M_Invalid,
        M_Superinfo,
        M_Dirnode,
        M_Filenode,
        M_Usertable,
    };

  private:
    Type type;

  public:
    UUID uuid;
    inline Type get_type() const {
        return type;
    };
    // if buf == nullptr, returns required size
    // if size < required size, returns 0
    virtual size_t dump(void* buf, size_t size) const = 0;
    size_t dump_to_buffer(void* buf, size_t size) const;

    template <typename T>
    static std::enable_if_t<std::is_base_of<Metadata, T>::value, T*>
    create(const UUID& uuid, const void* buf = nullptr, size_t size = 0);

};

class Inode : public Metadata {
  public:
    ino_t ino;
    virtual void dump_stat(stat_buffer_t* buf) const = 0;
    virtual size_t nlink() const = 0;
};
