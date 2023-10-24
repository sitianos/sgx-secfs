#include <../app/volume.hpp>
#include <cstring>
#include <exception>
#include <fstream>

int main(int argc, char **argv) {
    if (argc < 2) {
        std::cerr << "volume config is required" << std::endl;
        return 1;
    }
    secfs::Volume vol = secfs::Volume::load_config(argv[1]);

    if (! vol.loaded()) {
        std::cerr << "failed to load volume config" << std::endl;
        return 1;
    }
    secfs::StorageAPI &api = vol.get_api_instance();

    if (api.init() != 1) {
        std::cerr << "failed to initialize storage API" << std::endl;
        return 1;
    }

    return 0;
}
