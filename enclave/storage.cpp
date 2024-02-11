#include "storage.hpp"

#include "volume.hpp"

#include <mbedtls/cipher.h>
#include <mbusafecrt.h>
#include <memory>
#include <mutex>
#include <sgx_trts.h>
#include <vector>

static bool encrypt_buffer(
    const uint8_t* iv, size_t iv_len, const uint8_t* aad, size_t ad_len, const uint8_t* src,
    size_t ssize, uint8_t* dst, size_t dsize, uint8_t* tag, size_t tag_len
) {
    const mbedtls_cipher_info_t* cipher_info;
    mbedtls_cipher_context_t ctx_enc;

    size_t out_len, total_len;

    if (dsize < ssize) {
        return false;
    }

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

    if (mbedtls_cipher_setkey(&ctx_enc, volkey, sizeof(volkey), MBEDTLS_ENCRYPT) != 0) {
        return false;
    }

    if (mbedtls_cipher_set_iv(&ctx_enc, iv, iv_len) != 0) {
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

    if (mbedtls_cipher_finish(&ctx_enc, dst + out_len, &out_len) != 0) {
        return false;
    }
    total_len += out_len;

    if (mbedtls_cipher_write_tag(&ctx_enc, tag, tag_len) != 0) {
        return false;
    }

    return true;
}

static bool decrypt_buffer(
    const uint8_t* iv, size_t iv_len, const uint8_t* aad, size_t ad_len, const uint8_t* src,
    size_t ssize, uint8_t* dst, size_t dsize, const uint8_t* tag, size_t tag_len
) {
    const mbedtls_cipher_info_t* cipher_info;
    mbedtls_cipher_context_t ctx_dec;

    size_t out_len, total_len;

    if (dsize < ssize) {
        return false;
    }

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

    if (mbedtls_cipher_setkey(&ctx_dec, volkey, sizeof(volkey), MBEDTLS_DECRYPT) != 0) {
        return false;
    }

    if (mbedtls_cipher_set_iv(&ctx_dec, iv, iv_len) != 0) {
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

#if ENABLE_DEBUG_PRINT
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
#endif

bool load_metadata(Metadata& metadata) {
    void* obuf;
    uint8_t* ibuf;
    ssize_t osize, bsize;
    char filename[40];
    uint8_t iv[12];
    uint8_t aad[16];
    uint8_t tag[16];
    sgx_status_t sgxstat;

    metadata.uuid.unparse(filename);
    metadata.uuid.dump(aad);

    sgxstat = ocall_load_file(filename, &obuf, &osize);
    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return false;
    }
    if (osize < 0) {
        printf("failed to fetch file\n");
        return false;
    }

    bsize = osize - sizeof(tag);
    memcpy(tag, obuf + bsize, sizeof(tag));

    ibuf = new uint8_t[bsize];
    memset(iv, 0, sizeof(iv));

    if (!decrypt_buffer(
            iv, sizeof(iv), aad, sizeof(aad), static_cast<uint8_t*>(obuf), bsize, ibuf, bsize, tag,
            sizeof(tag)
        )) {
        printf("failed to decrypt metadata\n");
        return false;
    }

    if (!metadata.load(ibuf, bsize)) {
        return false;
    }

    ocall_free(obuf);
    return true;
}

bool save_metadata(const Metadata& metadata) {
    uint8_t* ibuf;
    uint8_t* obuf;
    size_t isize, bsize;
    char filename[40];
    uint8_t iv[12];
    uint8_t aad[16];
    uint8_t tag[16];
    sgx_status_t sgxstat;

    metadata.uuid.dump(aad);
    isize = metadata.dump(nullptr, 0);
    ibuf = new uint8_t[isize];
    if (metadata.dump(ibuf, isize) == 0) {
        delete[] ibuf;
        return false;
    }

    bsize = isize + sizeof(tag);
    obuf = new uint8_t[bsize];
    memset(iv, 0, sizeof(iv));

    if (!encrypt_buffer(
            iv, sizeof(iv), aad, sizeof(aad), ibuf, isize, obuf, isize, tag, sizeof(tag)
        )) {
        delete[] ibuf;
        delete[] obuf;
        printf("failed to encrypt metadata\n");
        return false;
    }
    memcpy(obuf + isize, tag, sizeof(tag));
    delete[] ibuf;

    metadata.uuid.unparse(filename);
    sgxstat = ocall_save_file(filename, obuf, bsize);
    if (sgxstat != SGX_SUCCESS) {
        delete[] obuf;
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return false;
    }
    delete[] obuf;
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

static bool load_cache(const Chunk& chunk, std::vector<uint8_t>& data) {
    void* obuf;
    ssize_t size;
    char filename[40];
    uint8_t aad[16];
    sgx_status_t sgxstat;

    chunk.uuid.dump(aad);
    chunk.uuid.unparse(filename);

    sgxstat = ocall_load_file(filename, &obuf, &size);
    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return false;
    }
    if (size < 0) {
        printf("failed to fetch chunk\n");
        return false;
    }
    if (size > CHUNKSIZE) {
        printf("chunk size is bigger then %ld/n", CHUNKSIZE);
        return false;
    }

    data.resize(CHUNKSIZE);

    if (!decrypt_buffer(
            chunk.iv, sizeof(iv_t), aad, sizeof(aad), (uint8_t*)obuf, size, data.data(), CHUNKSIZE,
            chunk.tag, sizeof(tag_t)
        )) {
        printf("failed to decrypt chunk %s\n", filename);
        return false;
    }
    ocall_free(obuf);
    return true;
}

static bool save_cache(Chunk& chunk, const std::vector<uint8_t>& data) {
    char filename[40];
    uint8_t aad[16];
    sgx_status_t sgxstat;
    std::unique_ptr<uint8_t[]> obuf = std::make_unique<uint8_t[]>(CHUNKSIZE);

    memset(chunk.iv, 0, sizeof(iv_t));
    chunk.uuid.dump(aad);
    chunk.uuid.unparse(filename);

    if (!encrypt_buffer(
            chunk.iv, sizeof(iv_t), aad, sizeof(aad), data.data(), data.size(), obuf.get(),
            CHUNKSIZE, chunk.tag, sizeof(tag_t)
        )) {
        printf("failed to encrypt chunk %s\n", filename);
        return false;
    }

    sgxstat = ocall_save_file(filename, obuf.get(), CHUNKSIZE);
    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return false;
    }

    return true;
}

bool load_chunk(ChunkStore& store, ChunkCache& cache) {
    if (!store.get_erase_by_key(cache.chunk().uuid, cache)) {
        return load_cache(cache.chunk(), cache.data);
    }
    return true;
}

bool save_chunk(ChunkStore& store, ChunkCache&& cache) {
    char filename[40];
    cache.chunk().uuid.unparse(filename);
    printf("push chunk %s\n", filename);
    ChunkStore::iterator iter;
    iter = store.push_get_back(std::move(cache));
    size_t lsize;
    while ((lsize = store.size()) > MAX_CACHE_NUM) {
        // std::lock_guard<std::mutex> lock(test_mutex);
        ChunkCache old_cache = store.get_pop_front();
        old_cache.chunk().uuid.unparse(filename);
        printf(
            "pop chunk %s idx=%ld\told_idx=%ld\tremain=%ld(%ld)\n", filename, cache.chunk_idx,
            old_cache.chunk_idx, lsize, store.size()
        );
        if (old_cache.modified) {
            std::shared_ptr<Filenode> fn;
            if (!(fn = old_cache.filenode.lock())) {
                printf("old cache is expired\n");
                return false;
            }
            Chunk chunk;
            {
                std::lock_guard<std::mutex> lock(fn->mutex);
                chunk.uuid = old_cache.chunk().uuid;
            }
            if (!save_cache(chunk, old_cache.data)) {
                printf("failed to save old cache\n");
                return false;
            }
            {
                std::lock_guard<std::mutex> lock(fn->mutex);
                old_cache.chunk() = chunk;
            }
        }
    }
    return true;
}

bool flush_chunk(ChunkStore& store, Chunk& chunk) {
    ChunkStore::iterator cache_iter;

    cache_iter = store.findbykey(chunk.uuid);
    if (cache_iter == store.end()) {
        return true;
    }

    ChunkCache cache = store.get_erase(cache_iter);

    if (cache.modified) {
        char filename[40];
        chunk.uuid.unparse(filename);
        printf("flush chunk %s\n", filename);

        if (!save_cache(chunk, cache.data)) {
            return false;
        }
    }
    return true;
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
