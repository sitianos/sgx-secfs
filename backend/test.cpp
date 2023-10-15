#include <cstring>
#include <exception>
#include <fstream>

#include "local.hpp"

int main(int argc, char **argv) {
    secfs::StorageAPI *api = new secfs::LocalStorage();
    std::ifstream ff(argc > 1 ? argv[1] : "volume.json");
    if (!ff) {
        std::cerr << "failed to open" << std::endl;
        return 1;
    }
    json config;
    try {
        config = json::parse(ff);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl;
        delete api;
        return 1;
    }
    if (api->load_config(config) != 1) {
        std::cerr << "failed to load configuration file" << std::endl;
        delete api;
        return 1;
    }
    if (api->init() != 1) {
        std::cerr << "failed to initialize instance" << std::endl;
        delete api;
        return 1;
    }
    const char *content = "hello";
    ssize_t wsize = api->set_content("hello", content, strlen(content));
    if (wsize < 0) {
        std::cerr << "failed to write to file" << std::endl;
        delete api;
        return 1;
    }
    std::cout << "write " << wsize << " bytes" << std::endl;

    char buf[64];
    ssize_t rsize = api->get_content("hello", buf, sizeof(buf));
    if (rsize < 0) {
        std::cerr << "failed to read from file" << std::endl;
        delete api;
        return 1;
    }
    std::cout << "read " << rsize << " bytes" << std::endl;

    if (!api->remove_file("hello")) {
        std::cout << "failed to remove file" << std::endl;
        delete api;
        return 1;
    }
    delete api;
    return 0;
}
