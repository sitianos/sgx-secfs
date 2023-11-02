#include "local.hpp"

#include <iostream>

namespace secfs {

int LocalStorage::init() {
    std::error_code ec;
    fs::file_status result = fs::status(base_dir, ec);
    if (ec) {
        std::cerr << base_dir << ": " << ec.message() << std::endl;
        return 0;
    } else if (result.type() == fs::file_type::none) {
        std::cerr << "no such directory " << base_dir << std::endl;
        return 0;
    } else if (result.type() != fs::file_type::directory) {
        std::cerr << base_dir << " is not directory" << std::endl;
        return 0;
    }
    return 1;
}

int LocalStorage::destroy() {
    return 1;
}

ssize_t LocalStorage::set_content(const char* filename, const void* buf, size_t size) {
    fs::path file = base_dir / filename;
    std::ofstream ofs(file);
    if (!ofs) {
        std::cerr << "failed to open " << file << std::endl;
        return -1;
    }
    ofs.write(static_cast<const char*>(buf), size);
    return size;
}

ssize_t LocalStorage::get_content(const char* filename, void* buf, size_t size) {
    fs::path file = base_dir / filename;
    std::ifstream ifs(file);
    if (!ifs) {
        std::cerr << "failed to open " << file << std::endl;
        return -1;
    }
    ifs.read(static_cast<char*>(buf), size);
    return ifs.gcount();
}
ssize_t LocalStorage::get_size(const char* filename) {
    std::error_code ec;
    fs::path file = base_dir / filename;
    ssize_t size = fs::file_size(file, ec);
    if (ec) {
        std::cerr << "get size of " << file << ": " << ec.message() << std::endl;
        return -1;
    }
    return size;
}

int LocalStorage::remove_file(const char* filename) {
    fs::path file = base_dir / filename;
    return fs::remove(file) ? 1 : 0;
}

LocalStorage LocalStorage::load_config(const json& config) {
    LocalStorage ret;
    ret.is_loaded = false;
    if (!config.is_object()) {
        std::cerr << "storage config is not object" << std::endl;
        return ret;
    }
    if (config.count("base_dir") == 0) {
        std::cerr << "no base_dir field" << std::endl;
        return ret;
    }
    ret.base_dir = fs::path(config["base_dir"]);
    ret.is_loaded = true;
    return ret;
}

} // namespace secfs
