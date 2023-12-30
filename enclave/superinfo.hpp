#pragma once

#include "metadata.hpp"

class Superinfo : public Metadata {
  private:
  public:
    Superinfo() = default;
    Superinfo(const superinfo_buffer_t* buf);
    Superinfo(const void* buf);
    ~Superinfo() override = default;

    bool load(const void* buf, size_t bsize) override;
    size_t dump(void* buf, size_t bsize) const override;

    UUID root_dirnode;
    UUID user_table;
    hash_t hash_root_dirnode;
    hash_t hash_user_table;
    ino_t max_ino;
};
