#pragma once

#include "metadata.hpp"

#include <string>
#include <vector>

class Dirnode : public Inode {
  private:
  public:
    class Dirent {
      public:
        Dirent() = default;
        Dirent(const dirent_t& dent);
        ino_t ino;
        std::string name;
        dirent_type_t type;
        UUID uuid;
        void dump(dirent_t& dent) const;
    };
    std::string name;
    std::vector<Dirent> dirent;

    Dirnode() = default;
    Dirnode(const dirnode_buffer_t& buf);

    size_t dump(void* buf, size_t size) const override;
    void dump_stat(stat_buffer_t* buf) const override;
    size_t nlink() const override;
};
