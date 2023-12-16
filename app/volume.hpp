#pragma once

#include "backend/local.hpp"
#include "backend/storage.hpp"
#include "sgx_urts.h"

#include <nlohmann/json.hpp>
#include <mbedtls/ecp.h>
#include <string>
#include <filesystem>

namespace secfs {
class Volume {
  private:
    Volume();
    StorageAPI* __load_api_instance(json config);
    StorageAPI* api;
    bool is_loaded;
    std::filesystem::path enclave_path;
    std::filesystem::path pubkey_path;
    std::filesystem::path key_path;
    std::filesystem::path base_dir;

  public:
    Volume(const Volume& vol) = delete;
    Volume(Volume&& vol) = delete;
    Volume& operator=(const Volume& vol) = delete;
    Volume& operator=(Volume&& vol) = delete;
    sgx_enclave_id_t eid;
    mbedtls_ecp_keypair pubkey;
    mbedtls_ecp_keypair key;

    ~Volume();
    StorageAPI& get_api_instance();
    int init_api_instance();
    int init_enclave();
    int load_pubkey();
    int load_key();
    inline bool loaded() {
        return is_loaded;
    }
    int load_config(const char* config_file);
    static Volume& get_instance();
};

extern Volume& global_vol;

} // namespace secfs

