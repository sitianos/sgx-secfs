#include "enclave.hpp"
#include "enclave_u.h"
#include "volume.hpp"

#include <cstdio>

using secfs::global_vol;

void ocall_load_file(const char* filename, void** content, ssize_t* size) {
    secfs::StorageAPI& api = global_vol.get_api_instance();
    *size = api.get_size(filename);
    if (*size < 0) {
        *size = -1;
        *content = nullptr;
        return;
    }
    *content = malloc(*size);
    ssize_t getsize = api.get_content(filename, *content, *size);
    if (*size < 0) {
        free(*content);
        *size = -1;
        *content = nullptr;
        return;
    }
    *size = getsize;
}

void ocall_save_file(const char* filename, void* content, size_t size) {
    secfs::StorageAPI& api = global_vol.get_api_instance();
    api.set_content(filename, content, size);
}

void ocall_remove_file(const char* filename, int* err) {
    secfs::StorageAPI& api = global_vol.get_api_instance();
    if (api.remove_file(filename))
        *err = 0;
    else
        *err = 1;
}

void ocall_free(void* mem) {
    free(mem);
}

void ocall_print_string(const char* str) {
    printf("\e[31m%s\e[0m", str);
    fflush(stdout);
}

void ocall_print_sgx_error(sgx_status_t sgxstat) {
    printf("\e[31m%s\n\e[0m", enclave_err_msg(sgxstat));
}
