#include "dirnode.hpp"
#include "enclave_t.h"
#include "filenode.hpp"
#include "metadata.hpp"
#include "storage.hpp"
#include "superinfo.hpp"
#include "volume.hpp"

#include <algorithm>
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <exception>
#include <mbusafecrt.h>

int ecall_fs_lookup(ino_t parent, const char* name, ino_t* ino, stat_buffer_t* statbuf) {
    *ino = 0;

    decltype(inode_map)::iterator iter = inode_map.find(parent);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    std::shared_ptr<Dirnode> parent_dn;
    if (!(parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second))) {
        return ENOENT;
    }
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
                ino_p = dir_p;
            } else if (dent->type == T_DT_REG) {
                Filenode* file_p = load_metadata<Filenode>(dent->uuid);
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

int ecall_fs_mkdir(
    ino_t parent, const char* name, mode_t mode, fuse_ino_t* ino, struct stat_buffer_t* statbuf
) {
    *ino = 0;
    decltype(inode_map)::iterator iter = inode_map.find(parent);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    std::shared_ptr<Dirnode> parent_dn;
    if (!(parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second))) {
        return ENOTDIR;
    }
    for (decltype(Dirnode::dirent)::iterator dent = parent_dn->dirent.begin();
         dent != parent_dn->dirent.end(); dent++) {
        if (dent->name == name) {
            return EEXIST;
        }
    }
    UUID new_uuid = UUID::gen_rand();
    std::shared_ptr<Dirnode> new_dn(Metadata::create<Dirnode>(new_uuid));
    new_dn->ino = superinfo->max_ino;
    new_dn->name = name;
    new_dn->dirent.resize(0);

    Dirnode::Dirent new_dirent;
    new_dirent.ino = superinfo->max_ino;
    new_dirent.name = name;
    new_dirent.type = T_DT_DIR;
    new_dirent.uuid = new_uuid;
    parent_dn->dirent.push_back(new_dirent);
    save_metadata(parent_dn.get());
    // inode_map.erase(iter);

    inode_map[superinfo->max_ino] = new_dn;
    *ino = superinfo->max_ino;
    superinfo->max_ino++;
    new_dn->dump_stat(statbuf);

    save_metadata(superinfo.get());
    save_metadata(new_dn.get());
    return 0;
}

int ecall_fs_unlink(fuse_ino_t parent, const char* name) {
    decltype(inode_map)::iterator iter = inode_map.find(parent);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    std::shared_ptr<Dirnode> parent_dn;
    if (!(parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second))) {
        return ENOTDIR;
    }
    for (decltype(Dirnode::dirent)::iterator dent = parent_dn->dirent.begin();
         dent != parent_dn->dirent.end(); dent++) {
        if (dent->name == name) {
            if (dent->type == T_DT_DIR) {
                return EISDIR;
            }
            Filenode* file_p = load_metadata<Filenode>(dent->uuid);
            if (file_p == nullptr) {
                printf("failed to load filenode\n");
                return EACCES;
            }
            if (!remove_metadata(dent->uuid)) {
                printf("failed to remove metadata\n");
                return EACCES;
            }
            for (Filenode::Chunk& chunk : file_p->chunks) {
                if (!remove_chunk(chunk.uuid)) {
                    printf("failed to remove chunk\n");
                }
            }

            delete file_p;

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
    std::shared_ptr<Dirnode> parent_dn;
    if (!(parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second))) {
        return ENOTDIR;
    }
    for (decltype(Dirnode::dirent)::iterator dent = parent_dn->dirent.begin();
         dent != parent_dn->dirent.end(); dent++) {
        if (dent->name == name) {
            if (dent->type != T_DT_DIR) {
                return ENOTDIR;
            }
            Dirnode* dir_p = load_metadata<Dirnode>(dent->uuid);
            if (dir_p == nullptr) {
                printf("failed to load dirnode\n");
                return EACCES;
            }
            if (dir_p->dirent.size() != 0) {
                return ENOTEMPTY;
            }
            if (!remove_metadata(dent->uuid)) {
                printf("failed to remove metadata\n");
                return EACCES;
            }
            delete dir_p;

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

int ecall_fs_open(fuse_ino_t ino, open_flag_t flags) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return EACCES;
    }
    std::shared_ptr<Filenode> fn;
    if (!(fn = std::dynamic_pointer_cast<Filenode>(iter->second))) {
        return EACCES;
    }
    // here is permission check
    if (flags & OF_TRUNC) {
        for (Filenode::Chunk& chunk : fn->chunks) {
            if (!remove_chunk(chunk.uuid)) {
                printf("failed to remove chunk\n");
            }
        }
        fn->chunks.resize(0);
        fn->size = 0;
    }
    return 0;
}

int ecall_fs_read(fuse_ino_t ino, char* buf, off_t offset, size_t* size) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return EINVAL;
    }
    std::shared_ptr<Filenode> fn;
    if (!(fn = std::dynamic_pointer_cast<Filenode>(iter->second))) {
        return EINVAL;
    }

    *size = std::min(*size, fn->size - offset);
    size_t chunk_st = offset / CHUNKSIZE;
    size_t chunk_en = (offset + *size + CHUNKSIZE - 1) / CHUNKSIZE;
    if (chunk_en > fn->chunks.size()) {
        printf("filesize is larger than local chunks\n");
        printf(
            "chunk_en=%ld, fn->chunks.size=%ld fn->size=%ld\n", chunk_en, fn->chunks.size(),
            fn->size
        );
        return EINVAL;
    }
    chunk_en = std::min(chunk_en, fn->chunks.size());
    size_t wsize = 0;

    for (size_t idx = chunk_st; idx < chunk_en; idx++) {
        Filenode::Chunk& chunk = fn->chunks[idx];
        off_t off_st = (idx == chunk_st) ? offset % CHUNKSIZE : 0;
        off_t off_en = (idx == chunk_en - 1) ? (offset + *size - 1) % CHUNKSIZE + 1 : CHUNKSIZE;

        if (chunk.mem == nullptr) {
            chunk.allocate();
            ssize_t rsize = load_chunk(chunk);
            if (rsize < 0) {
                chunk.deallocate();
                return EINVAL;
            }
            if (rsize > CHUNKSIZE) {
                printf("fetched chunk is larger (%ld) than CHUNKSIZE\n", rsize);
                return EINVAL;
            }
        }
        memcpy_verw_s(buf + wsize, *size - wsize, chunk.mem + off_st, off_en - off_st);
        wsize += off_en - off_st;
        chunk.deallocate();
    }
    if (wsize != *size) {
        printf("write size and read size does not match");
        return EINVAL;
    }
    return 0;
}

int ecall_fs_write(fuse_ino_t ino, const char* buf, off_t offset, size_t* size) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return EINVAL;
    }
    std::shared_ptr<Filenode> fn;
    if (!(fn = std::dynamic_pointer_cast<Filenode>(iter->second))) {
        return EINVAL;
    }

    size_t chunk_st = offset / CHUNKSIZE;
    size_t chunk_en = (offset + *size + CHUNKSIZE - 1) / CHUNKSIZE;

    size_t prev_size = fn->chunks.size();
    if (chunk_en > fn->chunks.size()) {
        fn->chunks.resize(chunk_en);
        for (size_t idx = prev_size; idx < chunk_en; idx++) {
            fn->chunks[idx].uuid = UUID::gen_rand();
        }
    }
    size_t wsize = 0;
    for (size_t idx = chunk_st; idx < chunk_en; idx++) {
        Filenode::Chunk& chunk = fn->chunks[idx];
        off_t off_st = (idx == chunk_st) ? offset % CHUNKSIZE : 0;
        off_t off_en = (idx == chunk_en - 1) ? (offset + *size - 1) % CHUNKSIZE + 1 : CHUNKSIZE;

        chunk.allocate();
        if (idx < prev_size && (off_st != 0 || off_en != CHUNKSIZE)) {
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

        memcpy_verw_s(chunk.mem + off_st, CHUNKSIZE - off_st, buf + wsize, off_en - off_st);
        chunk.modified = true;
        wsize += off_en - off_st;
        if (off_en == CHUNKSIZE) {
            if (save_chunk(chunk) < 0) {
                printf("failed to save chunk\n");
                break;
            }
            chunk.deallocate();
        }
    }
    *size = wsize;
    fn->size = std::max(fn->size, offset + wsize);
    return 0;
}

int ecall_fs_flush(fuse_ino_t ino) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return EINVAL;
    }
    std::shared_ptr<Filenode> fn;
    if (!(fn = std::dynamic_pointer_cast<Filenode>(iter->second))) {
        return EINVAL;
    }
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
        chunk.deallocate();
    }
    return 0;
}

int ecall_fs_get_dirent_size(fuse_ino_t ino, size_t* entnum) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    std::shared_ptr<Dirnode> parent_dn;
    if (!(parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second))) {
        return ENOTDIR;
    }
    *entnum = parent_dn->dirent.size();
    return 0;
}

int ecall_fs_get_dirent(fuse_ino_t ino, dirent_t* buf, size_t count) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    std::shared_ptr<Dirnode> parent_dn;
    if (!(parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second))) {
        return ENOTDIR;
    }
    size_t i = 0;
    for (Dirnode::Dirent& dent : parent_dn->dirent) {
        dent.dump(&buf[i]);
        i++;
        if (i >= count)
            break;
    }
    return 0;
}

int ecall_fs_access(fuse_ino_t ino, int mask) {
    auto iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    return 0;
}

int ecall_fs_create(
    fuse_ino_t parent, const char* name, mode_t mode, fuse_ino_t* ino, struct stat_buffer_t* statbuf
) {
    *ino = 0;
    auto iter = inode_map.find(parent);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    std::shared_ptr<Dirnode> parent_dn;
    if (!(parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second))) {
        return ENOTDIR;
    }

    for (decltype(Dirnode::dirent)::iterator dent = parent_dn->dirent.begin();
         dent != parent_dn->dirent.end(); dent++) {
        if (dent->name == name) {
            return EEXIST;
        }
    }
    UUID new_uuid = UUID::gen_rand();
    std::shared_ptr<Filenode> new_fn(Metadata::create<Filenode>(new_uuid));
    new_fn->ino = superinfo->max_ino;
    new_fn->size = 0;

    Dirnode::Dirent new_dirent;
    new_dirent.ino = superinfo->max_ino;
    new_dirent.name = name;
    new_dirent.type = T_DT_REG;
    new_dirent.uuid = new_uuid;
    parent_dn->dirent.push_back(new_dirent);
    save_metadata(parent_dn.get());

    inode_map[superinfo->max_ino] = new_fn;
    *ino = superinfo->max_ino;
    superinfo->max_ino++;
    new_fn->dump_stat(statbuf);

    save_metadata(superinfo.get());
    save_metadata(new_fn.get());

    return 0;
}
