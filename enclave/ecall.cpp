#include "dirnode.hpp"
#include "enclave.hpp"
#include "enclave_t.h"
#include "filenode.hpp"
#include "superinfo.hpp"
#include "volume.hpp"

#include <errno.h>
#include <mbusafecrt.h>
#include <stdarg.h>
#include <stdio.h>

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

static bool decrypt_buffer(Metadata::Type type, const UUID& uuid, void* obuf, const void* ibuf,
                           size_t isize) {
    memcpy_verw_s(obuf, isize, ibuf, isize);
    return true;
}

static bool remove_metadata(const UUID& uuid) {
    char filename[40];
    sgx_status_t status;

    uuid.unparse(filename);
    status = ocall_remove_file(filename);
    if (status != SGX_SUCCESS) {
        printf("SGX Error in %s(): %s\n", __func__, enclave_err_msg(status));
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

    status = ocall_fetch_file(filename, &buf, &size);
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
    status = ocall_dump_file(filename, buf, size);
    if (status != SGX_SUCCESS) {
        free(buf);
        printf("SGX Error in %s(): %s\n", __func__, enclave_err_msg(status));
        return false;
    }
    free(buf);
    return true;
}

void ecall_print() {
    sgx_status_t status;
    printf("test string %d\n", 10);
    // void* buf;
    // ssize_t size;
    // status = ocall_fetch_file("testfile", &buf, &size);
    // if (status != SGX_SUCCESS) {
    //     printf("SGX Error in %s(): %s\n", __func__, enclave_err_msg(status));
    //     return;
    // }
    // if (size < 0) {
    //     printf("failed to fetch testfile\n");
    //     return;
    // }
    // printf("get %ld bytes\n", size);
    // char* ebuf = (char*)malloc(size);
    // memcpy_s(ebuf, size, buf, size);
    // status = ocall_dump_file("dumpfile", ebuf, size);
    // if (status != SGX_SUCCESS) {
    //     printf("SGX Error in %s(): %s\n", __func__, enclave_err_msg(status));
    //     return;
    // }
}

void ecall_debug() {
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

    dirnode_map[1] = std::shared_ptr<Dirnode>(root);
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
}

int ecall_fs_lookup(ino_t parent, const char* name, ino_t* ino, stat_buffer_t* statbuf) {
    *ino = 0;

    decltype(dirnode_map)::iterator iter = dirnode_map.find(parent);
    if (iter == dirnode_map.end()) {
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
            if(ino_p == nullptr) {
                printf("failed to load metadata\n");
                delete ino_p;
                return ENONET;
            }

            ino_p->dump_stat(statbuf);
            dirnode_map[dirnode_ino] = std::shared_ptr<Inode>(ino_p);

            *ino = dirnode_ino;
            dirnode_ino++;
            return 0;
        }
    }
    printf("%s is not found\n", name);
    return ENOENT;
}

int ecall_fs_getattr(ino_t ino, stat_buffer_t* statbuf) {
    decltype(dirnode_map)::iterator iter = dirnode_map.find(ino);
    if (iter == dirnode_map.end()) {
        return ENOENT;
    }
    iter->second->dump_stat(statbuf);
    return 0;
}

int ecall_fs_mkdir(ino_t parent, const char* name, mode_t mode, fuse_ino_t* ino,
                   struct stat_buffer_t* statbuf) {
    *ino = 0;
    decltype(dirnode_map)::iterator iter = dirnode_map.find(parent);
    if (iter == dirnode_map.end()) {
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
    new_dn->ino = 30;
    new_dn->name = name;
    new_dn->dirent.resize(0);

    Dirnode::Dirent new_dirent;
    new_dirent.ino = 30;
    new_dirent.name = name;
    new_dirent.type = T_DT_DIR;
    new_dirent.uuid = new_uuid;
    parent_dn->dirent.push_back(new_dirent);
    save_metadata(parent_dn.get());
    // dirnode_map.erase(iter);

    dirnode_map[dirnode_ino] = new_dn;
    *ino = dirnode_ino;
    dirnode_ino++;
    new_dn->dump_stat(statbuf);

    save_metadata(new_dn.get());
    return 0;
}

int ecall_fs_rmdir(fuse_ino_t parent, const char* name) {
    decltype(dirnode_map)::iterator iter = dirnode_map.find(parent);
    if (iter == dirnode_map.end()) {
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
            if(dir_p){
                if(dir_p->dirent.size() != 0) {
                    return ENOTEMPTY;
                }
                if (!remove_metadata(dent->uuid)) {
                    printf("failed to remove metadata\n");
                    return EACCES;
                }
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

int ecall_fs_get_dirent(ino_t ino, dirent_t* buf, size_t count, ssize_t* getcount) {
    auto iter = dirnode_map.find(ino);
    if (iter == dirnode_map.end()) {
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
