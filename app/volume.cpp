#include "volume.hpp"

#include "enclave.hpp"
#include "enclave_u.h"

#include <iostream>
#include <mbedtls/base64.h>
#include <mbedtls/ctr_drbg.h>
#include <mbedtls/entropy.h>
#include <mbedtls/pk.h>

namespace secfs {
Volume::Volume() : api(nullptr), is_loaded(false), eid(0) {
    mbedtls_ecp_keypair_init(&key);
    mbedtls_ecp_keypair_init(&pubkey);
}

StorageAPI* Volume::__load_api_instance(json storage_config) {
    std::string type;
    if (storage_config.count("type") == 0 || !storage_config["type"].is_string()) {
        std::cerr << "no key field" << std::endl;
        return api = nullptr;
    }
    if (storage_config.count("config") == 0 || !storage_config["config"].is_object()) {
        std::cerr << "no config field" << std::endl;
        return api = nullptr;
    }
    type = storage_config["type"];
    if (type == "local") {
        api = new LocalStorage(LocalStorage::load_config(storage_config["config"]));
    } else {
        std::cerr << "invalid storage type" << std::endl;
        return api = nullptr;
    }
    if (!api) {
        std::cerr << "huh?" << std::endl;
        return api;
    }
    if (!api->loaded()) {
        delete api;
        return api = nullptr;
    }
    return api;
}

bool Volume::__init_enclave() {
    if (config.count("enclave_path") == 0) {
        std::cerr << "enclave path is not provided" << std::endl;
        return false;
    }
    if (!config["enclave_path"].is_string()) {
        std::cerr << "invalid enclave_path field" << std::endl;
        return false;
    }
    enclave_path = base_dir / config["enclave_path"];

    sgx_status_t ret;
    ret = sgx_create_enclave(enclave_path.c_str(), SGX_DEBUG_FLAG, NULL, NULL, &eid, NULL);
    if (ret != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(ret) << std::endl;
        return false;
    }
    return true;
}

StorageAPI& Volume::get_api_instance() {
    return *api;
}

bool Volume::__init_api_instance() {
    if (config.count("storage") == 0 || !config["storage"].is_object()) {
        std::cerr << "no storage field" << std::endl;
        return false;
    }
    if (__load_api_instance(config["storage"]) == nullptr) {
        std::cerr << "failed to load storage config" << std::endl;
        return false;
    }

    return api->init() != 0;
}

bool Volume::__load_key() {
    if (config.count("key_path") == 0) {
        std::cerr << "path of private key is not provided" << std::endl;
        return false;
    }
    if (!config["key_path"].is_string()) {
        std::cerr << "invalid key_path field" << std::endl;
        return false;
    }
    key_path = base_dir / config["key_path"];

    mbedtls_pk_context pk_ctx;
    // mbedtls_entropy_context entropy;
    // mbedtls_ctr_drbg_context ctr_drbg;
    // const char* pers = "mbedtls_pk_decrypt";
    mbedtls_pk_init(&pk_ctx);
    // mbedtls_entropy_init(&entropy);
    // mbedtls_ctr_drbg_init(&ctr_drbg);

    // if (mbedtls_ctr_drbg_seed(&ctr_drbg, mbedtls_entropy_func, &entropy, (const unsigned
    // char*)pers,
    //                           strlen(pers)) != 0) {
    //     std::cerr << "failed to generate seed" << std::endl;
    //     return 0;
    // }
    if (mbedtls_pk_parse_keyfile(
            &pk_ctx, key_path.c_str(), ""
            // , mbedtls_ctr_drbg_random, &ctr_drbg
        ) != 0) {
        std::cerr << "failed to parse " << key_path << std::endl;
        return false;
    }

    if (mbedtls_pk_get_type(&pk_ctx) != MBEDTLS_PK_ECKEY) {
        std::cerr << "given key is not ECDSA private key" << std::endl;
        return false;
    }
    if (mbedtls_ecp_copy(&key.MBEDTLS_PRIVATE(Q), &mbedtls_pk_ec(pk_ctx)->MBEDTLS_PRIVATE(Q)) !=
            0 ||
        mbedtls_ecp_group_copy(
            &key.MBEDTLS_PRIVATE(grp), &mbedtls_pk_ec(pk_ctx)->MBEDTLS_PRIVATE(grp)
        ) != 0 ||
        mbedtls_mpi_copy(&key.MBEDTLS_PRIVATE(d), &mbedtls_pk_ec(pk_ctx)->MBEDTLS_PRIVATE(d)) !=
            0) {
        std::cerr << "failed to copy private key" << std::endl;
        return false;
    }
    mbedtls_pk_free(&pk_ctx);
    return true;
}

bool Volume::__load_pubkey() {
    if (config.count("public_key_path") == 0) {
        std::cerr << "path of public key is not provided" << std::endl;
        return false;
    }
    if (!config["public_key_path"].is_string()) {
        std::cerr << "invalid public_key_path field" << std::endl;
        return false;
    }
    pubkey_path = base_dir / config["public_key_path"];

    mbedtls_pk_context pk_ctx;
    mbedtls_pk_init(&pk_ctx);
    if (mbedtls_pk_parse_public_keyfile(&pk_ctx, pubkey_path.c_str())) {
        std::cerr << "failed to parse " << pubkey_path << std::endl;
        return false;
    }
    if (mbedtls_pk_get_type(&pk_ctx) != MBEDTLS_PK_ECKEY) {
        std::cerr << "given key is not ECDSA private key" << std::endl;
        return false;
    }
    if (mbedtls_ecp_copy(&pubkey.MBEDTLS_PRIVATE(Q), &mbedtls_pk_ec(pk_ctx)->MBEDTLS_PRIVATE(Q)) !=
            0 ||
        mbedtls_ecp_group_copy(
            &pubkey.MBEDTLS_PRIVATE(grp), &mbedtls_pk_ec(pk_ctx)->MBEDTLS_PRIVATE(grp)
        ) != 0) {
        std::cerr << "failed to copy public key" << std::endl;
        return false;
    }
    mbedtls_pk_free(&pk_ctx);
    return true;
}

bool Volume::mount_volume() {
    if (config.count("superinfo") == 0) {
        std::cerr << "UUID of superinfo is not provided" << std::endl;
        return false;
    }
    if (!config["superinfo"].is_string()) {
        std::cerr << "invalid superinfo field" << std::endl;
        return false;
    }
    std::string superinfo = config["superinfo"];
    uuid_t sp_uuid;

    if (config.count("uid") == 0) {
        std::cerr << "UID is not provided" << std::endl;
        return false;
    }
    if (!config["uid"].is_number_integer()) {
        std::cerr << "invalid UID field" << std::endl;
        return false;
    }
    int uid = config["uid"];

    if (uuid_parse(superinfo.c_str(), sp_uuid) != 0) {
        std::cerr << "UUID of supernode is invalid" << std::endl;
        return false;
    }

    if (config.count("volume_key") == 0) {
        std::cerr << "volume key is not provided" << std::endl;
        return false;
    }
    if (!config["volume_key"].is_string()) {
        std::cerr << "invalid volume_key field" << std::endl;
        return false;
    }
    std::string encoded_sealed_volkey = config["volume_key"];
    std::unique_ptr<unsigned char[]> sealed_volkey;
    size_t volkey_size;
    if (mbedtls_base64_decode(
            nullptr, 0, &volkey_size, (const unsigned char*)encoded_sealed_volkey.c_str(),
            encoded_sealed_volkey.size()
        ) == MBEDTLS_ERR_BASE64_INVALID_CHARACTER) {
        std::cerr << "given volume key is invalid format" << std::endl;
        return false;
    }
    sealed_volkey = std::make_unique<unsigned char[]>(volkey_size);
    mbedtls_base64_decode(
        sealed_volkey.get(), volkey_size, &volkey_size,
        (const unsigned char*)encoded_sealed_volkey.c_str(), encoded_sealed_volkey.size()
    );

    if (!__load_key() || !__init_enclave() || !__init_api_instance()) {
        return false;
    }

    int err;
    sgx_status_t sgxstat;

    sgxstat = ecall_mount_volume(eid, &err, uid, &key, sp_uuid, sealed_volkey.get(), volkey_size);

    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        return false;
    } else if (err) {
        std::cerr << "failed to mount volume within enclave" << std::endl;
        return false;
    }

    return true;
}

bool Volume::create_volume() {
    int err;
    sgx_status_t sgxstat;
    uuid_t sp_uuid;
    char buf[37];

    std::unique_ptr<unsigned char[]> sealed_volkey;
    std::unique_ptr<char[]> encoded_sealed_volkey;
    size_t volkey_size, encoded_volkey_size;

    if (!__load_pubkey() || !__init_enclave() || !__init_api_instance()) {
        return false;
    }

    sgxstat = ecall_calc_volkey_size(eid, &volkey_size);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        return false;
    }

    sealed_volkey = std::make_unique<unsigned char[]>(volkey_size);

    sgxstat = ecall_create_volume(eid, &err, &pubkey, sp_uuid, sealed_volkey.get(), volkey_size);

    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        return false;
    } else if (err) {
        std::cerr << "failed to create volume within enclave" << std::endl;
        return false;
    }

    mbedtls_base64_encode(nullptr, 0, &encoded_volkey_size, nullptr, volkey_size);
    encoded_sealed_volkey = std::make_unique<char[]>(encoded_volkey_size);
    mbedtls_base64_encode(
        (unsigned char*)encoded_sealed_volkey.get(), encoded_volkey_size, &encoded_volkey_size,
        sealed_volkey.get(), volkey_size
    );

    uuid_unparse(sp_uuid, buf);
    config["superinfo"] = buf;
    config["uid"] = 1;
    config["volume_key"] = encoded_sealed_volkey.get();

    return true;
}

Volume::~Volume() {
    sgx_status_t sgxstat;
    if (api)
        delete api;
    if (eid) {
        sgxstat = sgx_destroy_enclave(eid);
        if (sgxstat != SGX_SUCCESS) {
            std::cerr << enclave_err_msg(sgxstat) << std::endl;
        }
    }
    mbedtls_ecp_keypair_free(&key);
    mbedtls_ecp_keypair_free(&pubkey);
}

bool Volume::load_config(const char* config_file) {
    std::ifstream ifs(config_file);
    if (!ifs) {
        std::cerr << "failed to open " << config_file << std::endl;
        return false;
    }
    try {
        config = secfs::json::parse(ifs);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    base_dir = std::filesystem::path(config_file).parent_path();

    is_loaded = true;
    return true;
}

bool Volume::dump_config(const char* config_file) {
    std::ofstream ofs(config_file);
    if (!ofs) {
        std::cerr << "failed to open " << config_file << std::endl;
        return false;
    }
    ofs << std::setw(4) << config;
    return true;
}

Volume& Volume::get_instance() {
    static Volume instance;
    return instance;
}

Volume& global_vol = Volume::get_instance();

} // namespace secfs
