#pragma once

#include "backend/local.hpp"
#include "backend/storage.hpp"
#include "sgx_urts.h"

#include <filesystem>
#include <mbedtls/ecp.h>
#include <nlohmann/json.hpp>
#include <string>
#include <uuid/uuid.h>

namespace secfs {
class Volume {
  private:
    Volume();
    StorageAPI* __load_api_instance(json config);
    bool __init_api_instance();
    bool __init_enclave();
    bool __load_pubkey();
    bool __load_key();
    StorageAPI* api;
    bool is_loaded;
    json config;
    std::filesystem::path enclave_path;
    std::filesystem::path pubkey_path;
    std::filesystem::path key_path;
    std::filesystem::path base_dir;

  public:
    Volume(const Volume& vol) = delete;
    Volume(Volume&& vol) = delete;
    ~Volume();
    Volume& operator=(const Volume& vol) = delete;
    Volume& operator=(Volume&& vol) = delete;
    sgx_enclave_id_t eid;
    mbedtls_ecp_keypair pubkey;
    mbedtls_ecp_keypair key;
    struct {
        int writeback;
        int debug;
    } options;

    StorageAPI& get_api_instance();
    inline bool loaded() {
        return is_loaded;
    }
    bool create_volume();
    bool mount_volume();
    bool load_config(const char* config_file);
    bool dump_config(const char* config_file);
    static Volume& get_instance();
};

extern Volume& global_vol;

} // namespace secfs
