#pragma once

#include "nlohmann/json.hpp"

#include <iostream>

namespace secfs {

using json = nlohmann::json;

class StorageAPI {
  private:
  protected:
    bool is_loaded;

  public:
    virtual ~StorageAPI() = default;
    virtual int init() = 0;
    virtual int destroy() = 0;

    virtual ssize_t set_content(const char* filename, const void* buf, size_t size) = 0;
    virtual ssize_t get_content(const char* filename, void* buf, size_t size) = 0;
    virtual ssize_t get_size(const char* filename) = 0;
    virtual int remove_file(const char* filename) = 0;

    inline bool loaded() {
        return is_loaded;
    }
};

} // namespace secfs
