#include "volume.hpp"
#include "enclave_u.h"
#include "enclave.hpp"

#include <iostream>
#include <cstring>
#include <getopt.h>

using secfs::global_vol;

static int cmd_create_volume(int argc, char **argv) {
    struct option options[] = {
        {"config", required_argument, NULL, 'c'},
        {0, 0, NULL, 0}
    };
    int opt;
    int longindex;
    const char *volume_config = NULL;
    while((opt = getopt_long(argc, argv, "", options, &longindex)) != -1){
        switch(opt){
          case 'c':
            volume_config = optarg;
        }
    }
    if(! volume_config) {
        std::cerr << "configuration file is not given" << std::endl;
        return 1;
    }
    std::cout << "load " << volume_config << std::endl;

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

    int err;
    sgx_status_t sgxstat;
    uuid_t sp_uuid;

    sgxstat = ecall_create_volume(global_vol.eid, &err, &global_vol.pubkey, sp_uuid);

    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        return 1;
    } else if(err) {
        std::cerr << "failed to create volume within enclave" << std::endl;
        return 1;
    }
    std::cout << std::dec << std::endl;
    return 0;
}


int main(int argc, char **argv, char** envp) {
    (void)envp;
    if(std::strcpy(argv[1], "create")) {
        return cmd_create_volume(argc - 1, &argv[1]);
    }

}
