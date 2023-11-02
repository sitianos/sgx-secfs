#include <cstdio>
#include "volume.hpp"
#include "enclave.hpp"
#include "enclave_u.h"

using secfs::global_vol;

void ocall_fetch_file(const char *filename, void **content, ssize_t *size) {
    secfs::StorageAPI &api = global_vol.get_api_instance();
    *size = api.get_size(filename);
    if(*size < 0) {
        *size = -1;
        *content = nullptr;
        return;
    }
    *content = new char[*size];
    ssize_t getsize = api.get_content(filename, *content, *size);
    if(*size < 0) {
        delete[] *content;
        *size = -1;
        *content = nullptr;
        return;
    }
    *size = getsize;
}

void ocall_dump_file(const char* filename, void* content, size_t size) {
    secfs::StorageAPI &api = global_vol.get_api_instance();
    api.set_content(filename, content, size);
}

void ocall_remove_file(const char* filename) {
    secfs::StorageAPI &api = global_vol.get_api_instance();
    api.remove_file(filename);
}

void ocall_print_string(const char* str) {
    printf("%s", str);
    fflush(stdout);
}
