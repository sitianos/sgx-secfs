#include "enclave.hpp"
#include "enclave_u.h"
#include "volume.hpp"

#include <cstring>
#include <getopt.h>
#include <iostream>

using secfs::global_vol;

static int cmd_create_volume(int argc, char** argv) {
    struct option options[] = {
        {"config", required_argument, NULL, 'c'},
        {"new-config", required_argument, NULL, 'n'},
        {0, 0, NULL, 0}
    };
    int opt;
    int longindex;
    const char* volume_config = NULL;
    const char* new_volume_config = NULL;
    while ((opt = getopt_long(argc, argv, "c:n:", options, &longindex)) != -1) {
        switch (opt) {
        case 'c':
            volume_config = optarg;
            break;
        case 'n':
            new_volume_config = optarg;
            break;
        }
    }
    if (!volume_config) {
        std::cerr << "configuration file is not given" << std::endl;
        return 1;
    }
    if (!new_volume_config) {
        new_volume_config = volume_config;
    }

    if (!global_vol.load_config(volume_config)) {
        std::cerr << "volume is not loaded" << std::endl;
        return 1;
    }

    if (!global_vol.create_volume()) {
        std::cerr << "failed to create volume" << std::endl;
        return 1;
    }
    if (!global_vol.dump_config(new_volume_config)) {
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
