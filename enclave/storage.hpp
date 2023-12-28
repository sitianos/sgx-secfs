#include "enclave_t.h"
#include "filenode.hpp"
#include "metadata.hpp"

void hexdump(const void* bytes, size_t len, char* out);

int print_hex(const void* bytes, size_t len);

int printf(const char* fmt, ...);

int print_sgx_err(sgx_status_t sgxstat);

bool save_metadata(const Metadata* metadata);

template <typename T>
T* load_metadata(const UUID& uuid) {
    void* buf;
    ssize_t size;
    char filename[40];
    sgx_status_t sgxstat;

    uuid.unparse(filename);

    sgxstat = ocall_load_file(filename, &buf, &size);
    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return nullptr;
    }
    if (size < 0) {
        printf("failed to fetch file\n");
        return nullptr;
    }

    // decryption here

    T* metadata = Metadata::create<T>(uuid, buf, size);
    ocall_free(buf);
    return metadata;
}

bool remove_metadata(const UUID& uuid);

ssize_t load_chunk(Filenode::Chunk& chunk);

ssize_t save_chunk(Filenode::Chunk& chunk);

bool remove_chunk(const UUID& uuid);
