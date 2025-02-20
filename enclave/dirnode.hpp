#pragma once

#include "metadata.hpp"

#include <string>
#include <vector>

class Dirent {
  public:
    Dirent() = default;
    Dirent(const dirent_t* dent);
    Dirent(const dirent_t& dent);

    void dump(dirent_t* dent) const;

    ino_t ino;
    std::string name;
    dirent_type_t type;
    UUID uuid;
};

class Dirnode : public Inode {
  private:
  public:
    using Inode::Inode;
    ~Dirnode() override = default;

    bool load(const void* buf, size_t bsize) override;
    size_t dump(void* buf, size_t bsize) const override;
    void dump_stat(stat_buffer_t* buf) const override;
    size_t nlink() const override;

    std::string name;
    std::vector<Dirent> dirent;
};
