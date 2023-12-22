#include "enclave.hpp"
#include "enclave_u.h"
#include "volume.hpp"

#include <cstring>
#include <getopt.h>
#include <iostream>

using secfs::global_vol;

static int cmd_create_volume(int argc, char** argv) {
    struct option options[] = {{"config", required_argument, NULL, 'c'}, {0, 0, NULL, 0}};
    int opt;
    int longindex;
    const char* volume_config = NULL;
    while ((opt = getopt_long(argc, argv, "", options, &longindex)) != -1) {
        switch (opt) {
        case 'c':
            volume_config = optarg;
        }
    }
    if (!volume_config) {
        std::cerr << "configuration file is not given" << std::endl;
        return 1;
    }

    global_vol.load_config(volume_config);

    if (!global_vol.loaded()) {
        std::cerr << "volume is not loaded" << std::endl;
        return 1;
    }
    if (!global_vol.init_enclave()) {
        std::cerr << "failed to initialize enclave" << std::endl;
        return 1;
    }
    if (global_vol.init_api_instance() != 1) {
        std::cerr << "failed to initialize API instance" << std::endl;
        return 1;
    }
    if (global_vol.load_pubkey() != 1) {
        std::cerr << "failed to load public key" << std::endl;
        return 1;
    }

    if (!global_vol.create_volume()) {
        std::cerr << "failed to create volume" << std::endl;
        return 1;
    }
    if (!global_vol.dump_config(volume_config)) {
        std::cerr << "failed to dump to volume configuration file" << std::endl;
        return 1;
    }

    return 0;
}

int main(int argc, char** argv, char** envp) {
    (void)envp;
    if (std::strcpy(argv[1], "create")) {
        return cmd_create_volume(argc - 1, &argv[1]);
    }
}
