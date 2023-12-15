#include "enclave.hpp"
#include "enclave_t.h"
#include "filenode.hpp"
#include "metadata.hpp"

int printf(const char* fmt, ...);

bool save_metadata(const Metadata* metadata);

template <typename T>
T* load_metadata(const UUID& uuid) {
    void* buf;
    ssize_t size;
    char filename[40];
    sgx_status_t status;

    uuid.unparse(filename);

    status = ocall_load_file(filename, &buf, &size);
    if (status != SGX_SUCCESS) {
        printf("SGX Error in %s(): 0x%4x %s\n", __func__, status, enclave_err_msg(status));
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
