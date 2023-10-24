#include "volume.hpp"
#include "enclave.hpp"

#include <iostream>

namespace secfs {
Volume::Volume() : api(nullptr), is_loaded(false), eid(0) {
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

int Volume::init_enclave() {
    sgx_status_t ret = SGX_ERROR_UNEXPECTED;
    ret = sgx_create_enclave(enclave_path.c_str(), SGX_DEBUG_FLAG, NULL, NULL, &eid, NULL);
    if (ret != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(ret) << std::endl;
        return -1;
    }
    return 0;
}

StorageAPI& Volume::get_api_instance() {
    return *api;
}

int Volume::init_api_instance() {
    if (!api) {
        std::cerr << "Storage API is not loaded" << std::endl;
        return 0;
    }
    return api->init();
}

Volume::~Volume() {
    if (api)
        delete api;
}

Volume Volume::load_config(const char* config_file) {
    json config;
    std::ifstream ifs(config_file);
    Volume vol;
    if (!ifs) {
        std::cerr << "failed to open " << config_file << std::endl;
        return vol;
    }
    try {
        config = secfs::json::parse(ifs);
    } catch (std::exception& e) {
        std::cerr << e.what() << std::endl;
        return vol;
    }
    if (config.count("enclave_path") == 0 || !config["enclave_path"].is_string()) {
        std::cerr << "invalid enclave_path field" << std::endl;
        return vol;
    }
    vol.enclave_path = config["enclave_path"];
    if (config.count("storage") == 0 || !config["storage"].is_object()) {
        std::cerr << "no storage field" << std::endl;
        return vol;
    }
    if (vol.__load_api_instance(config["storage"]) == nullptr) {
        std::cerr << "failed to load storage config" << std::endl;
        return vol;
    }
    vol.is_loaded = true;
    return vol;
}

} // namespace secfs
