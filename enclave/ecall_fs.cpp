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
            std::shared_ptr<Inode> inode;
            iter = inode_map.find(dent->ino);

            if (iter != inode_map.end()) {
                inode = iter->second;
            } else {
                Inode* ino_p;
                if (dent->type == T_DT_DIR) {
                    Dirnode* dir_p = new Dirnode(dent->uuid);
                    ino_p = dir_p;
                } else if (dent->type == T_DT_REG) {
                    Filenode* file_p = new Filenode(dent->uuid);
                    ino_p = file_p;
                } else {
                    printf("only file and directory lookup is supported\n");
                    return ENOENT;
                }
                if (!load_metadata(*ino_p)) {
                    printf("failed to load metadata\n");
                    delete ino_p;
                    return ENONET;
                }
                inode = inode_map[dent->ino] = std::shared_ptr<Inode>(ino_p);
            }

            if (inode->ino != dent->ino) {
                printf("loaded metadata inode number does not match one in parent metadata\n");
                return ENOENT;
            }

            inode->dump_stat(statbuf);
            inode->refcount++;

            *ino = dent->ino;
            return 0;
        }
    }
    return ENOENT;
}

int ecall_fs_forget(ino_t ino, size_t nlookup) {
    decltype(inode_map)::iterator iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    std::shared_ptr<Inode> inode = iter->second;
    int remain = inode->refcount - nlookup;
    if (remain <= 0) {
        inode_map.erase(iter);
    } else {
        inode->refcount = remain;
    }
    return remain;
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
    std::shared_ptr<Dirnode> new_dn = std::make_shared<Dirnode>(new_uuid);
    new_dn->ino = superinfo->max_ino;
    new_dn->name = name;
    new_dn->dirent.resize(0);

    Dirent new_dirent;
    new_dirent.ino = superinfo->max_ino;
    new_dirent.name = name;
    new_dirent.type = T_DT_DIR;
    new_dirent.uuid = new_uuid;
    parent_dn->dirent.push_back(new_dirent);
    save_metadata(*parent_dn);
    // inode_map.erase(iter);

    inode_map[superinfo->max_ino] = new_dn;
    *ino = superinfo->max_ino;
    superinfo->max_ino++;
    new_dn->dump_stat(statbuf);
    new_dn->refcount++;

    save_metadata(*superinfo);
    save_metadata(*new_dn);
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
            std::shared_ptr<Filenode> file_p = std::make_shared<Filenode>(dent->uuid);
            if (!load_metadata(*file_p)) {
                printf("failed to load filenode\n");
                return EACCES;
            }
            if (!remove_metadata(dent->uuid)) {
                printf("failed to remove metadata\n");
                return EACCES;
            }
            for (Chunk& chunk : file_p->chunks) {
                if (chunk.uuid != UUID() && !remove_chunk(chunk.uuid)) {
                    printf("failed to remove chunk\n");
                }
            }
            parent_dn->dirent.erase(dent);
            if (!save_metadata(*parent_dn)) {
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
            std::shared_ptr<Dirnode> dir_p = std::make_shared<Dirnode>(dent->uuid);
            if (!load_metadata(*dir_p)) {
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
            parent_dn->dirent.erase(dent);
            if (!save_metadata(*parent_dn)) {
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
    decltype(inode_map)::iterator iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return EACCES;
    }
    std::shared_ptr<Filenode> fn;
    if (!(fn = std::dynamic_pointer_cast<Filenode>(iter->second))) {
        return EACCES;
    }
    // here is permission check
    if (flags & OF_TRUNC) {
        for (Chunk& chunk : fn->chunks) {
            if (chunk.uuid != UUID() && !remove_chunk(chunk.uuid)) {
                printf("failed to remove chunk\n");
            }
        }
        fn->chunks.resize(0);
        fn->size = 0;
    }
    return 0;
}

int ecall_fs_read(fuse_ino_t ino, char* buf, off_t offset, size_t* size) {
    decltype(inode_map)::iterator iter = inode_map.find(ino);
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
        printf(
            "filesize is larger than local chunks\n"
            "chunk_en=%ld, fn->chunks.size=%ld fn->size=%ld\n",
            chunk_en, fn->chunks.size(), fn->size
        );
        return EINVAL;
    }
    chunk_en = std::min(chunk_en, fn->chunks.size());
    size_t wsize = 0;
    sizeof(Chunk);

    for (size_t idx = chunk_st; idx < chunk_en; idx++) {
        Chunk& chunk = fn->chunks[idx];
        off_t off_st = (idx == chunk_st) ? offset % CHUNKSIZE : 0;
        off_t off_en = (idx == chunk_en - 1) ? (offset + *size - 1) % CHUNKSIZE + 1 : CHUNKSIZE;

        if (fn->chunks[idx].uuid == UUID()) {
            memset(buf + wsize, 0, off_en - off_st);
            wsize += off_en - off_st;
            continue;
        }
        ChunkCache cache(CHUNKSIZE, fn, idx);

        if (!load_chunk(local_cache, cache)) {
            printf("failed to load chunk idx = %ld\n", idx);
            return EINVAL;
        }

        memcpy_verw_s(buf + wsize, *size - wsize, cache.data.data() + off_st, off_en - off_st);
        wsize += off_en - off_st;
        if (!save_chunk(local_cache, std::move(cache))) {
            printf("failed to save chunk\n");
            return EINVAL;
        }
    }
    if (wsize != *size) {
        printf("write size and read size does not match");
        return EINVAL;
    }
    return 0;
}

int ecall_fs_write(fuse_ino_t ino, const char* buf, off_t offset, size_t* size) {
    decltype(inode_map)::iterator iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return EINVAL;
    }
    std::shared_ptr<Filenode> fn;
    if (!(fn = std::dynamic_pointer_cast<Filenode>(iter->second))) {
        return EINVAL;
    }

    size_t chunk_st = offset / CHUNKSIZE;
    size_t chunk_en = (offset + *size + CHUNKSIZE - 1) / CHUNKSIZE;

    fn->mutex.lock();
    size_t prev_size = fn->chunks.size();
    off_t last_surplus = (fn->size + CHUNKSIZE - 1) % CHUNKSIZE;
    if (chunk_en > prev_size) {
        fn->chunks.resize(chunk_en);
    }
    fn->mutex.unlock();

    size_t wsize = 0;
    for (size_t idx = chunk_st; idx < chunk_en; idx++) {
        off_t off_st = (idx == chunk_st) ? offset % CHUNKSIZE : 0;
        off_t off_en = (idx == chunk_en - 1) ? (offset + *size - 1) % CHUNKSIZE + 1 : CHUNKSIZE;
        off_t off_chunk_en = (idx == prev_size - 1) ? last_surplus : CHUNKSIZE;

        ChunkCache cache(CHUNKSIZE, fn, idx);

        if (fn->chunks[idx].uuid == UUID()) {
            fn->chunks[idx].uuid = UUID::gen_rand();
        } else if ((off_st != 0 || off_en != off_chunk_en)) {
            if (!load_chunk(local_cache, cache)) {
                printf("failed to load chunk for writing\n");
            }
        }
        if (cache.data.size() != CHUNKSIZE) {
            printf("idx=%ld size is not chunk size %ld\n", idx, cache.data.size());
        }
        if (cache.chunk_idx != idx) {
            printf("idx=%ld idx does not match %ld\n", idx, cache.chunk_idx);
        }

        memcpy_verw_s(cache.data.data() + off_st, CHUNKSIZE - off_st, buf + wsize, off_en - off_st);
        cache.modified = true;
        wsize += off_en - off_st;
        if (!save_chunk(local_cache, std::move(cache))) {
            printf("failed to save chunk idx = %ld\n", idx);
            return EINVAL;
        }
    }
    *size = wsize;
    fn->mutex.lock();
    fn->size = std::max(fn->size, offset + wsize);
    fn->mutex.unlock();
    return 0;
}

int ecall_fs_flush(fuse_ino_t ino) {
    decltype(inode_map)::iterator iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return EINVAL;
    }
    std::shared_ptr<Filenode> fn;
    if (!(fn = std::dynamic_pointer_cast<Filenode>(iter->second))) {
        return EINVAL;
    }

    for (Chunk& chunk : fn->chunks) {
        if (!flush_chunk(local_cache, chunk)) {
            printf("failed to flush chunk\n");
            return EIO;
        }
    }
    if (!save_metadata(*fn))
        return EIO;

    return 0;
}

int ecall_fs_get_dirent_size(fuse_ino_t ino, size_t* entnum) {
    decltype(inode_map)::iterator iter = inode_map.find(ino);
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
    decltype(inode_map)::iterator iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    std::shared_ptr<Dirnode> parent_dn;
    if (!(parent_dn = std::dynamic_pointer_cast<Dirnode>(iter->second))) {
        return ENOTDIR;
    }
    size_t i = 0;
    for (Dirent& dent : parent_dn->dirent) {
        dent.dump(&buf[i]);
        i++;
        if (i >= count)
            break;
    }
    return 0;
}

int ecall_fs_access(fuse_ino_t ino, int mask) {
    decltype(inode_map)::iterator iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return ENOENT;
    }
    return 0;
}

int ecall_fs_create(
    fuse_ino_t parent, const char* name, mode_t mode, fuse_ino_t* ino, struct stat_buffer_t* statbuf
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
    std::shared_ptr<Filenode> new_fn = std::make_shared<Filenode>(new_uuid);
    new_fn->ino = superinfo->max_ino;
    new_fn->size = 0;

    Dirent new_dirent;
    new_dirent.ino = superinfo->max_ino;
    new_dirent.name = name;
    new_dirent.type = T_DT_REG;
    new_dirent.uuid = new_uuid;
    parent_dn->dirent.push_back(new_dirent);
    save_metadata(*parent_dn);

    inode_map[superinfo->max_ino] = new_fn;
    *ino = superinfo->max_ino;
    superinfo->max_ino++;
    new_fn->dump_stat(statbuf);
    new_fn->refcount++;

    save_metadata(*superinfo);
    save_metadata(*new_fn);

    return 0;
}

int ecall_fs_fallocate(fuse_ino_t ino, int mode, off_t offset, size_t length){
    decltype(inode_map)::iterator iter = inode_map.find(ino);
    if (iter == inode_map.end()) {
        return EACCES;
    }
    std::shared_ptr<Filenode> fn;
    if (!(fn = std::dynamic_pointer_cast<Filenode>(iter->second))) {
        return EACCES;
    }
    // here is permission check
    size_t chunk_st = offset / CHUNKSIZE;
    size_t chunk_en = (offset + length + CHUNKSIZE - 1) / CHUNKSIZE;

    fn->mutex.lock();
    size_t prev_size = fn->chunks.size();
    if (chunk_en > prev_size) {
        fn->chunks.resize(chunk_en);
    }
    fn->size = std::max(fn->size, offset + length);
    fn->mutex.unlock();
    return 0;
}
