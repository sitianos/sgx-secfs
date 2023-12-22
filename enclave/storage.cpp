#include "storage.hpp"

#include <mbusafecrt.h>

static bool decrypt_buffer(const UUID& uuid, void* obuf, const void* ibuf,
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

int print_sgx_err(sgx_status_t sgxstat) {
    ocall_print_sgx_error(sgxstat);
}

bool save_metadata(const Metadata* metadata) {
    void* buf;
    size_t size;
    char filename[40];
    sgx_status_t sgxstat;

    size = metadata->dump(nullptr, 0);
    buf = malloc(size);
    if (metadata->dump(buf, size) == 0) {
        free(buf);
        return false;
    }

    // encryption here

    metadata->uuid.unparse(filename);
    sgxstat = ocall_save_file(filename, buf, size);
    if (sgxstat != SGX_SUCCESS) {
        free(buf);
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return false;
    }
    free(buf);
    return true;
}

bool remove_metadata(const UUID& uuid) {
    char filename[40];
    sgx_status_t sgxstat;
    int err;

    uuid.unparse(filename);
    sgxstat = ocall_remove_file(filename, &err);
    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return false;
    }
    if (err) {
        return false;
    }
    return true;
}

ssize_t load_chunk(Filenode::Chunk& chunk) {
    void* buf;
    ssize_t size;
    char filename[40];
    sgx_status_t sgxstat;

    chunk.uuid.unparse(filename);

    sgxstat = ocall_load_file(filename, &buf, &size);
    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
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

ssize_t save_chunk(Filenode::Chunk& chunk) {
    void* obuf;
    char filename[40];
    sgx_status_t sgxstat;

    try {
        obuf = new char[CHUNKSIZE];
    } catch(std::exception &e) {
        printf("%s\n", e.what());
        return -1;
    }

    // encryption here
    memcpy_verw_s(obuf, CHUNKSIZE, chunk.mem, CHUNKSIZE);

    chunk.uuid.unparse(filename);

    sgxstat = ocall_save_file(filename, obuf, CHUNKSIZE);
    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return -1;
    }
    chunk.modified = false;
    
    delete obuf;
    return CHUNKSIZE;
}

bool remove_chunk(const UUID& uuid) {
    char filename[40];
    sgx_status_t sgxstat;
    int err;

    uuid.unparse(filename);
    sgxstat = ocall_remove_file(filename, &err);
    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return false;
    }
    if (err) {
        return false;
    }
    return true;
}
