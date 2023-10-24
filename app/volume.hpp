#pragma once

#include "backend/local.hpp"
#include "backend/storage.hpp"
#include "sgx_urts.h"

#include <nlohmann/json.hpp>
#include <string>

namespace secfs {
class Volume {
  private:
    StorageAPI* __load_api_instance(json config);
    StorageAPI* api;
    bool is_loaded;
    std::string enclave_path;

  public:
    sgx_enclave_id_t eid;
    Volume();
    ~Volume();
    StorageAPI& get_api_instance();
    int init_api_instance();
    int init_enclave();
    inline bool loaded() {
        return is_loaded;
    }
    static Volume load_config(const char* config_file);
};
} // namespace secfs
