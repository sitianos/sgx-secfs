#include "storage.hpp"

#include "volume.hpp"

#include <mbedtls/cipher.h>
#include <mbusafecrt.h>
#include <memory>

static bool encrypt_buffer(
    const uint8_t* src, size_t ssize, const uint8_t* aad, size_t ad_len, uint8_t* dst, size_t dsize,
    uint8_t* tag, size_t tag_len
) {
    const mbedtls_cipher_info_t* cipher_info;
    mbedtls_cipher_context_t ctx_enc;

    std::vector<uint8_t> iv;
    size_t out_len, total_len;

    mbedtls_cipher_init(&ctx_enc);
    std::unique_ptr<mbedtls_cipher_context_t, decltype(&mbedtls_cipher_free)> ctx_enc_ptr(
        &ctx_enc, mbedtls_cipher_free
    );

    cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_GCM);
    if (cipher_info == nullptr) {
        return false;
    }

    if (mbedtls_cipher_setup(&ctx_enc, cipher_info) != 0) {
        return false;
    }

    iv.assign(mbedtls_cipher_get_iv_size(&ctx_enc), 0);

    if (mbedtls_cipher_setkey(&ctx_enc, volkey, sizeof(volkey), MBEDTLS_ENCRYPT) != 0) {
        return false;
    }

    if (mbedtls_cipher_set_iv(&ctx_enc, iv.data(), iv.size()) != 0) {
        return false;
    }

    if (mbedtls_cipher_reset(&ctx_enc) != 0) {
        return false;
    }

    if (mbedtls_cipher_update_ad(&ctx_enc, aad, ad_len) != 0) {
        return false;
    }

    if (mbedtls_cipher_update(&ctx_enc, src, ssize, dst, &out_len) != 0) {
        return false;
    }
    total_len = out_len;

    if (0 != mbedtls_cipher_finish(&ctx_enc, dst + out_len, &out_len) != 0) {
        return false;
    }
    total_len += out_len;

    if (mbedtls_cipher_write_tag(&ctx_enc, tag, tag_len) != 0) {
        return false;
    }

    return true;
}

static bool decrypt_buffer(
    const uint8_t* src, size_t ssize, const uint8_t* aad, size_t ad_len, uint8_t* dst, size_t dsize,
    const uint8_t* tag, size_t tag_len
) {
    const mbedtls_cipher_info_t* cipher_info;
    mbedtls_cipher_context_t ctx_dec;

    std::vector<uint8_t> iv;
    size_t out_len, total_len;

    mbedtls_cipher_init(&ctx_dec);
    std::unique_ptr<mbedtls_cipher_context_t, decltype(&mbedtls_cipher_free)> ctx_dec_ptr(
        &ctx_dec, mbedtls_cipher_free
    );

    cipher_info = mbedtls_cipher_info_from_type(MBEDTLS_CIPHER_AES_256_GCM);
    if (cipher_info == nullptr) {
        return false;
    }

    if (mbedtls_cipher_setup(&ctx_dec, cipher_info) != 0) {
        return false;
    }

    iv.assign(mbedtls_cipher_get_iv_size(&ctx_dec), 0);

    if (mbedtls_cipher_setkey(&ctx_dec, volkey, sizeof(volkey), MBEDTLS_DECRYPT) != 0) {
        return false;
    }

    if (mbedtls_cipher_set_iv(&ctx_dec, iv.data(), iv.size()) != 0) {
        return false;
    }

    if (mbedtls_cipher_reset(&ctx_dec) != 0) {
        return false;
    }

    if (mbedtls_cipher_update_ad(&ctx_dec, aad, ad_len) != 0) {
        return false;
    }

    if (mbedtls_cipher_update(&ctx_dec, src, ssize, dst, &out_len) != 0) {
        return false;
    }
    total_len = out_len;

    if (mbedtls_cipher_finish(&ctx_dec, dst + out_len, &out_len) != 0) {
        return false;
    }
    total_len += out_len;

    if (mbedtls_cipher_check_tag(&ctx_dec, tag, tag_len) != 0) {
        return false;
    }

    return true;
}

void hexdump(const void* bytes, size_t len, char* out) {
    for (int i = 0; i < len; i++)
        snprintf(out + i * 2, 3, "%02x", *((char*)bytes + i));
}

int print_hex(const void* bytes, size_t len) {
    char buf[len * 2 + 2];
    hexdump(bytes, len, buf);
    buf[len * 2] = '\n';
    buf[len * 2 + 1] = '\0';
    ocall_print_string(buf);
    return 0;
}

int printf(const char* fmt, ...) {
    const size_t bufsize = 1024;
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

bool load_metadata(Metadata& metadata) {
    void* buf;
    ssize_t size;
    char filename[40];
    sgx_status_t sgxstat;

    metadata.uuid.unparse(filename);

    sgxstat = ocall_load_file(filename, &buf, &size);
    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return false;
    }
    if (size < 0) {
        printf("failed to fetch file\n");
        return false;
    }

    // decryption here

    if (!metadata.load(buf, size)) {
        return false;
    }

    ocall_free(buf);
    return true;
}

bool save_metadata(const Metadata& metadata) {
    uint8_t* buf;
    size_t size;
    char filename[40];
    sgx_status_t sgxstat;

    size = metadata.dump(nullptr, 0);
    buf = new uint8_t[size];
    if (metadata.dump(buf, size) == 0) {
        delete[] buf;
        return false;
    }

    // encryption here

    metadata.uuid.unparse(filename);
    sgxstat = ocall_save_file(filename, buf, size);
    if (sgxstat != SGX_SUCCESS) {
        delete[] buf;
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return false;
    }
    delete[] buf;
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
    } catch (std::exception& e) {
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
