#include "storage.hpp"

#include "enclave.hpp"
#include "enclave_t.h"

#include <mbusafecrt.h>

static bool decrypt_buffer(Metadata::Type type, const UUID& uuid, void* obuf, const void* ibuf,
                           size_t isize) {
    memcpy_verw_s(obuf, isize, ibuf, isize);
    return true;
}

int printf(const char* fmt, ...) {
    const size_t bufsize = 256;
    char buf[bufsize] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, bufsize, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
    return 0;
}

bool save_metadata(const Metadata* metadata) {
    void* buf;
    size_t size;
    char filename[40];
    sgx_status_t status;

    size = metadata->dump_to_buffer(nullptr, 0);
    buf = malloc(size);
    if (metadata->dump_to_buffer(buf, size) == 0) {
        free(buf);
        return false;
    }

    // encryption here

    metadata->uuid.unparse(filename);
    status = ocall_save_file(filename, buf, size);
    if (status != SGX_SUCCESS) {
        free(buf);
        printf("SGX Error in %s(): %s\n", __func__, enclave_err_msg(status));
        return false;
    }
    free(buf);
    return true;
}

ssize_t load_chunk(Filenode::Chunk& chunk) {
    void* buf;
    ssize_t size;
    char filename[40];
    sgx_status_t status;

    chunk.uuid.unparse(filename);

    status = ocall_load_file(filename, &buf, &size);
    if (status != SGX_SUCCESS) {
        printf("SGX Error in %s(): 0x%4x %s\n", __func__, status, enclave_err_msg(status));
        return -1;
    }
    if (size < 0) {
        printf("failed to fetch chunk\n");
        return -1;
    }

    chunk.allocate();

    // decryption here
    memcpy_verw_s(chunk.mem, CHUNKSIZE, buf, size);
    chunk.modified = false;

    ocall_free(buf);
    return size;
}

bool remove_metadata(const UUID& uuid) {
    char filename[40];
    sgx_status_t status;
    int err;

    uuid.unparse(filename);
    status = ocall_remove_file(filename, &err);
    if (status != SGX_SUCCESS) {
        printf("SGX Error in %s(): %s\n", __func__, enclave_err_msg(status));
        return false;
    }
    if (err) {
        return false;
    }
    return true;
}

ssize_t save_chunk(Filenode::Chunk& chunk) {
    void* obuf;
    char filename[40];
    sgx_status_t status;

    obuf = malloc(CHUNKSIZE);

    // encryption here
    memcpy_verw_s(obuf, CHUNKSIZE, chunk.mem, CHUNKSIZE);

    chunk.uuid.unparse(filename);

    status = ocall_save_file(filename, obuf, CHUNKSIZE);
    if (status != SGX_SUCCESS) {
        printf("SGX Error in %s(): 0x%4x %s\n", __func__, status, enclave_err_msg(status));
        return -1;
    }
    chunk.modified = false;
    free(obuf);
    return CHUNKSIZE;
}

bool remove_chunk(const UUID& uuid) {
    char filename[40];
    sgx_status_t status;
    int err;

    uuid.unparse(filename);
    status = ocall_remove_file(filename, &err);
    if (status != SGX_SUCCESS) {
        printf("SGX Error in %s(): %s\n", __func__, enclave_err_msg(status));
        return false;
    }
    if (err) {
        return false;
    }
    return true;
}
