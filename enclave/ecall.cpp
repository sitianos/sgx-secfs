#include "dirnode.hpp"
#include "enclave.hpp"
#include "enclave_t.h"
#include "filenode.hpp"
#include "superinfo.hpp"
#include "volume.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <mbedtls/sha256.h>
#include <mbusafecrt.h>

static int printf(const char* fmt, ...) {
    const size_t bufsize = 256;
    char buf[bufsize] = {'\0'};
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(buf, bufsize, fmt, ap);
    va_end(ap);
    ocall_print_string(buf);
    return 0;
}

static void hexdump(const void* bytes, size_t len, char* out) {
    for (int i = 0; i < len; i++)
        snprintf(out + i * 2, 3, "%02x", *((char*)bytes + i));
}

static bool decrypt_buffer(Metadata::Type type, const UUID& uuid, void* obuf, const void* ibuf,
                           size_t isize) {
    memcpy_verw_s(obuf, isize, ibuf, isize);
    return true;
}

static bool remove_metadata(const UUID& uuid) {
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

template <typename T>
static T* load_metadata(const UUID& uuid) {
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

static bool save_metadata(const Metadata* metadata) {
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

static bool remove_chunk(const UUID& uuid) {
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

static ssize_t load_chunk(Filenode::Chunk& chunk) {
    void* obuf;
    ssize_t size;
    char filename[40];
    sgx_status_t status;

    chunk.uuid.unparse(filename);

    status = ocall_load_file(filename, &obuf, &size);
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
    memcpy_verw_s(chunk.mem, CHUNKSIZE, obuf, size);
    chunk.modified = false;

    return size;
}

static ssize_t save_chunk(Filenode::Chunk& chunk) {
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

void ecall_debug() {
    unsigned char str[] = "hello";
    unsigned char hash[32];
    mbedtls_sha256(str, 5, hash, 0);
    char hex[65];
    hexdump(hash, 32, hex);
    printf("hash = %s\n", hex);

    uuid_t uid = {1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

    UUID uuid1(uid);
    Dirnode* root = Metadata::create<Dirnode>(uuid1);
    root->ino = 1;
    root->name = "";
    root->dirent.resize(2);

    root->dirent[0].ino = 2;
    root->dirent[0].name = "dir1";
    root->dirent[0].type = T_DT_DIR;
    uid[0] = 2;
    UUID uuid2(uid);
    root->dirent[0].uuid = uuid2;

    root->dirent[1].ino = 3;
    root->dirent[1].name = "file1";
    root->dirent[1].type = T_DT_REG;
    uid[0] = 3;
    UUID uuid3(uid);
    root->dirent[1].uuid = uuid3;

    Filenode* file1 = Metadata::create<Filenode>(uuid3);
    file1->ino = 3;
    file1->size = 0;
    save_metadata(file1);
    delete file1;

    inode_map[1] = std::shared_ptr<Dirnode>(root);
    save_metadata(root);

    Dirnode* dir1 = Metadata::create<Dirnode>(uuid2);
    dir1->ino = 2;
    dir1->name = "dir1";
    dir1->dirent.resize(2);

    dir1->dirent[0].ino = 4;
    dir1->dirent[0].name = "file2";
    dir1->dirent[0].type = T_DT_REG;
    uid[0] = 4;
    UUID uuid4(uid);
    dir1->dirent[0].uuid = uuid4;

    Filenode* file2 = Metadata::create<Filenode>(uuid4);
    file2->ino = 4;
    file2->size = 0;
    save_metadata(file2);
    delete file2;

    dir1->dirent[1].ino = 5;
    dir1->dirent[1].name = "file3";
    dir1->dirent[1].type = T_DT_REG;
    uid[0] = 5;
    UUID uuid5(uid);
    dir1->dirent[1].uuid = uuid5;

    Filenode* file3 = Metadata::create<Filenode>(uuid5);
    file3->ino = 5;
    file3->size = 0;
    save_metadata(file3);
    delete file3;

    save_metadata(dir1);
    delete dir1;

    max_ino = 6;
}

int ecall_fs_lookup(ino_t parent, const char* name, ino_t* ino, stat_buffer_t* statbuf) {
    *ino = 0;

    decltype(inode_map)::iterator iter = inode_map.find(parent);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    if (iter->second->get_type() != Metadata::M_Dirnode) {
        return ENOENT;
    }
    std::shared_ptr<Dirnode> parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second);
    char filename[36];
    void* ubuf;
    ssize_t fsize;
    sgx_status_t status;
    for (decltype(Dirnode::dirent)::iterator dent = parent_dn->dirent.begin();
         dent != parent_dn->dirent.end(); dent++) {
        if (dent->name == name) {
            Inode* ino_p;

            iter = inode_map.find(dent->ino);
            if (iter != inode_map.end()) {
                ino_p = iter->second.get();
                ino_p->dump_stat(statbuf);
                *ino = dent->ino;
                return 0;
            }

            if (dent->type == T_DT_DIR) {
                Dirnode* dir_p = load_metadata<Dirnode>(dent->uuid);
                // Dirnode* dir_p = Metadata::create<Dirnode>(dent->uuid, ubuf, fsize);
                ino_p = dir_p;
            } else if (dent->type == T_DT_REG) {
                Filenode* file_p = load_metadata<Filenode>(dent->uuid);
                // Filenode* file_p = Metadata::create<Filenode>(dent->uuid, ubuf, fsize);
                ino_p = file_p;
            } else {
                printf("only file and directory lookup is supported\n");
                return ENOENT;
            }
            if (ino_p == nullptr) {
                printf("failed to load metadata\n");
                delete ino_p;
                return ENONET;
            }

            ino_p->dump_stat(statbuf);
            inode_map[dent->ino] = std::shared_ptr<Inode>(ino_p);

            *ino = dent->ino;
            return 0;
        }
    }
    return ENOENT;
}

int ecall_fs_getattr(ino_t ino, stat_buffer_t* statbuf) {
    decltype(inode_map)::iterator iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    iter->second->dump_stat(statbuf);
    return 0;
}

int ecall_fs_mkdir(ino_t parent, const char* name, mode_t mode, fuse_ino_t* ino,
                   struct stat_buffer_t* statbuf) {
    *ino = 0;
    decltype(inode_map)::iterator iter = inode_map.find(parent);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    if (iter->second->get_type() != Metadata::M_Dirnode) {
        return ENOTDIR;
    }
    std::shared_ptr<Dirnode> parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second);
    for (decltype(Dirnode::dirent)::iterator dent = parent_dn->dirent.begin();
         dent != parent_dn->dirent.end(); dent++) {
        if (dent->name == name) {
            return EEXIST;
        }
    }
    UUID new_uuid;
    std::shared_ptr<Dirnode> new_dn(Metadata::create<Dirnode>(new_uuid));
    new_dn->ino = max_ino;
    new_dn->name = name;
    new_dn->dirent.resize(0);

    Dirnode::Dirent new_dirent;
    new_dirent.ino = max_ino;
    new_dirent.name = name;
    new_dirent.type = T_DT_DIR;
    new_dirent.uuid = new_uuid;
    parent_dn->dirent.push_back(new_dirent);
    save_metadata(parent_dn.get());
    // inode_map.erase(iter);

    inode_map[max_ino] = new_dn;
    *ino = max_ino;
    max_ino++;
    new_dn->dump_stat(statbuf);

    save_metadata(new_dn.get());
    return 0;
}

int ecall_fs_unlink(fuse_ino_t parent, const char* name) {
    decltype(inode_map)::iterator iter = inode_map.find(parent);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    if (iter->second->get_type() != Metadata::M_Dirnode) {
        return ENOTDIR;
    }
    std::shared_ptr<Dirnode> parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second);
    for (decltype(Dirnode::dirent)::iterator dent = parent_dn->dirent.begin();
         dent != parent_dn->dirent.end(); dent++) {
        if (dent->name == name) {
            if (dent->type == T_DT_DIR) {
                return EISDIR;
            }
            Filenode* file_p = load_metadata<Filenode>(dent->uuid);
            if (file_p) {
                if (!remove_metadata(dent->uuid)) {
                    printf("failed to remove metadata\n");
                    return EACCES;
                }
                for (decltype(Filenode::chunks)::iterator chunk = file_p->chunks.begin();
                     chunk != file_p->chunks.end(); chunk++) {
                    if (!remove_chunk(chunk->uuid)) {
                        printf("failed to remove chunk\n");
                    }
                }
            } else {
                printf("failed to load filenode\n");
                return EACCES;
            }

            parent_dn->dirent.erase(dent);
            if (!save_metadata(parent_dn.get())) {
                printf("failed to save metadata\n");
                return EACCES;
            }
            return 0;
        }
    }
    printf("%s is not found\n", name);
    return ENOENT;
}

int ecall_fs_rmdir(fuse_ino_t parent, const char* name) {
    decltype(inode_map)::iterator iter = inode_map.find(parent);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    if (iter->second->get_type() != Metadata::M_Dirnode) {
        return ENOTDIR;
    }
    std::shared_ptr<Dirnode> parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second);
    for (decltype(Dirnode::dirent)::iterator dent = parent_dn->dirent.begin();
         dent != parent_dn->dirent.end(); dent++) {
        if (dent->name == name) {
            if (dent->type != T_DT_DIR) {
                return ENOTDIR;
            }
            Dirnode* dir_p = load_metadata<Dirnode>(dent->uuid);
            if (dir_p) {
                if (dir_p->dirent.size() != 0) {
                    return ENOTEMPTY;
                }
                if (!remove_metadata(dent->uuid)) {
                    printf("failed to remove metadata\n");
                    return EACCES;
                }
            } else {
                printf("failed to load dirnode\n");
                return EACCES;
            }

            parent_dn->dirent.erase(dent);
            if (!save_metadata(parent_dn.get())) {
                printf("failed to save metadata\n");
                return EACCES;
            }
            return 0;
        }
    }
    printf("%s is not found\n", name);
    return ENOENT;
}

int ecall_fs_open(fuse_ino_t ino, int flags) {
    return 0;
}

int ecall_fs_read(fuse_ino_t ino, char* buf, size_t offset, size_t* size) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end() || iter->second->get_type() != Metadata::M_Filenode) {
        return EINVAL;
    }
    std::shared_ptr<Filenode> fn = std::dynamic_pointer_cast<Filenode>(iter->second);

    *size = std::min(*size, fn->size - offset);
    size_t chunk_st = offset / CHUNKSIZE;
    size_t chunk_en = (offset + *size + CHUNKSIZE - 1) / CHUNKSIZE;
    if (chunk_en > fn->chunks.size()) {
        printf("filesize is larger than local chunks\n");
        printf("chunk_en=%ld, fn->chunks.size=%ld fn->size=%ld\n", chunk_en, fn->chunks.size(),
               fn->size);
        return EINVAL;
    }
    chunk_en = std::min(chunk_en, fn->chunks.size());
    size_t wsize = 0;

    for (size_t idx = chunk_st; idx < chunk_en; idx++) {
        Filenode::Chunk& chunk = fn->chunks[idx];
        size_t off_st = (idx == chunk_st) ? offset % CHUNKSIZE : 0;
        size_t off_en = (idx == chunk_en - 1) ? (offset + *size - 1) % CHUNKSIZE + 1 : CHUNKSIZE;

        if (chunk.mem == nullptr) {
            chunk.allocate();
            ssize_t rsize = load_chunk(chunk);
            if (rsize < 0) {
                free(chunk.mem);
                chunk.mem = nullptr;
                return EINVAL;
            }
            if (rsize > CHUNKSIZE) {
                printf("fetched chunk is larger (%ld) than CHUNKSIZE\n", rsize);
                return EINVAL;
            }
        }
        memcpy_verw_s(buf + wsize, *size - wsize, chunk.mem + off_st, off_en - off_st);
        wsize += off_en - off_st;
    }
    if (wsize != *size) {
        printf("write size and read size does not match");
        return EINVAL;
    }
    return 0;
}

int ecall_fs_write(fuse_ino_t ino, const char* buf, size_t offset, size_t* size) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end() || iter->second->get_type() != Metadata::M_Filenode) {
        return EINVAL;
    }
    std::shared_ptr<Filenode> fn = std::dynamic_pointer_cast<Filenode>(iter->second);

    size_t chunk_st = offset / CHUNKSIZE;
    size_t chunk_en = (offset + *size + CHUNKSIZE - 1) / CHUNKSIZE;

    for (int i = 0; i < fn->chunks.size(); i++) {
        printf("idx=%ld, mem=%p\n", i, fn->chunks[i].mem);
    }

    if (chunk_en > fn->chunks.size()) {
        size_t prev_size = fn->chunks.size();
        printf("extend chunks from %ld to %ld\n", prev_size, chunk_en);
        fn->chunks.resize(chunk_en);
        for (size_t idx = prev_size; idx < chunk_en; idx++) {
            fn->chunks[idx].allocate();
        }
    }
    size_t wsize = 0;
    for (int i = 0; i < fn->chunks.size(); i++) {
        printf("idx=%ld, mem=%p\n", i, fn->chunks[i].mem);
    }

    for (size_t idx = chunk_st; idx < chunk_en; idx++) {
        Filenode::Chunk& chunk = fn->chunks[idx];
        size_t off_st = (idx == chunk_st) ? offset % CHUNKSIZE : 0;
        size_t off_en = (idx == chunk_en - 1) ? (offset + *size - 1) % CHUNKSIZE + 1 : CHUNKSIZE;
        printf("idx=%ld, st=%ld, en=%ld mem=%p\n", idx, off_st, off_en, chunk.mem);

        if (chunk.mem == nullptr) {
            chunk.allocate();
            if (off_st != 0 || off_en != CHUNKSIZE) {
                ssize_t rsize = load_chunk(chunk);
                if (rsize < 0) {
                    chunk.deallocate();
                    return EINVAL;
                }
                if (rsize > CHUNKSIZE) {
                    chunk.deallocate();
                    printf("fetched chunk is larger (%ld) than CHUNKSIZE\n", rsize);
                    return EINVAL;
                }
            }
        }

        memcpy_verw_s(chunk.mem + off_st, CHUNKSIZE - off_st, buf + wsize, off_en - off_st);
        chunk.modified = true;
        wsize += off_en - off_st;
    }
    *size = wsize;
    fn->size = std::max(fn->size, offset + wsize);
    return 0;
}

int ecall_fs_flush(fuse_ino_t ino) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end() || iter->second->get_type() != Metadata::M_Filenode) {
        return EBADF;
    }
    std::shared_ptr<Filenode> fn = std::dynamic_pointer_cast<Filenode>(iter->second);
    if (!save_metadata(fn.get()))
        return EIO;

    for (Filenode::Chunk& chunk : fn->chunks) {
        if (chunk.modified) {
            if (chunk.mem == nullptr) {
                printf("modified bit is set but memory is not allocated\n");
                continue;
            }
            if (save_chunk(chunk) < 0) {
                printf("failed to save chunk\n");
                continue;
            }
        }
    }
    return 0;
}

int ecall_fs_get_dirent(fuse_ino_t ino, dirent_t* buf, size_t count, ssize_t* getcount) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        *getcount = 0;
        return ENOENT;
    }
    if (iter->second->get_type() != Metadata::M_Dirnode) {
        *getcount = 0;
        return ENOTDIR;
    }
    std::shared_ptr<Dirnode> parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second);
    size_t i = 0;
    for (Dirnode::Dirent& dent : parent_dn->dirent) {
        dent.dump(buf[i]);
        i++;
        if (i >= count)
            break;
    }
    *getcount = i;
    return 0;
}

int ecall_fs_access(fuse_ino_t ino, int mask) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    return 0;
}

int ecall_fs_create(fuse_ino_t parent, const char* name, mode_t mode, fuse_ino_t* ino,
                    struct stat_buffer_t* statbuf) {
    *ino = 0;
    auto iter = inode_map.find(parent);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    if (iter->second->get_type() != Metadata::M_Dirnode) {
        return ENOTDIR;
    }
    std::shared_ptr<Dirnode> parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second);
    for (decltype(Dirnode::dirent)::iterator dent = parent_dn->dirent.begin();
         dent != parent_dn->dirent.end(); dent++) {
        if (dent->name == name) {
            return EEXIST;
        }
    }
    UUID new_uuid;
    std::shared_ptr<Filenode> new_fn(Metadata::create<Filenode>(new_uuid));
    new_fn->ino = max_ino;
    new_fn->size = 0;

    Dirnode::Dirent new_dirent;
    new_dirent.ino = max_ino;
    new_dirent.name = name;
    new_dirent.type = T_DT_REG;
    new_dirent.uuid = new_uuid;
    parent_dn->dirent.push_back(new_dirent);
    save_metadata(parent_dn.get());

    inode_map[max_ino] = new_fn;
    *ino = max_ino;
    max_ino++;
    new_fn->dump_stat(statbuf);

    save_metadata(new_fn.get());

    return 0;
}
