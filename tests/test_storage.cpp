#include <cstring>
#include <exception>
#include <fstream>

#include "../app/backend/local.hpp"

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "storage config is required" << std::endl;
        return 1;
    }
    std::ifstream ff(argv[1]);
    if (!ff) {
        std::cerr << "failed to open " << argv[1] << std::endl;
        return 1;
    }
    secfs::json config;
    try {
        config = secfs::json::parse(ff);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    secfs::LocalStorage lst = secfs::LocalStorage::load_config(config);
    secfs::StorageAPI &api = lst;
    if (! api.loaded()) {
        std::cerr << "failed to load configuration file" << std::endl;
        return 1;
    }
    if (api.init() != 1) {
        std::cerr << "failed to initialize instance" << std::endl;
        return 1;
    }
    const char *content = "hello";
    ssize_t wsize = api.set_content("hello", content, strlen(content));
    if (wsize < 0) {
        std::cerr << "failed to write to file" << std::endl;
        return 1;
    }
    std::cout << "write " << wsize << " bytes" << std::endl;

    char buf[64];
    ssize_t rsize = api.get_content("hello", buf, sizeof(buf));
    if (rsize < 0) {
        std::cerr << "failed to read from file" << std::endl;
        return 1;
    }
    std::cout << "read " << rsize << " bytes" << std::endl;

    if (!api.remove_file("hello")) {
        std::cout << "failed to remove file" << std::endl;
        return 1;
    }
    std::cout << "succeed in removeing" << std::endl;
    return 0;
}
