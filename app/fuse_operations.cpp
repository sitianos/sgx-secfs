#include "fuse_operations.hpp"

#include "bridge.hpp"
#include "enclave.hpp"
#include "enclave_u.h"
#include "volume.hpp"

#include <cstring>
#include <fuse_lowlevel.h>

static void copy_statbuf(struct stat& st, const struct stat_buffer_t& statbuf) {
    std::memset(&st, 0, sizeof(struct stat));
    st.st_ino = statbuf.ino;
    st.st_mode = statbuf.mode;
    st.st_nlink = statbuf.nlink;
    st.st_size = statbuf.size;
    st.st_mode |= 00777;
}

void secfs_init(void* userdata, struct fuse_conn_info* conn) {
    (void)userdata;
    (void)conn;
    if (secfs::global_vol.options.writeback && conn->capable & FUSE_CAP_WRITEBACK_CACHE) {
        conn->want |= FUSE_CAP_WRITEBACK_CACHE;
    }
}

void secfs_lookup(fuse_req_t req, fuse_ino_t parent, const char* name) {
    struct fuse_entry_param ep;
    fuse_ino_t ino;
    sgx_status_t sgxstat;
    int err;
    struct stat_buffer_t statbuf;

    sgxstat = ecall_fs_lookup(secfs::global_vol.eid, &err, parent, name, &ino, &statbuf);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else if (err) {
        fuse_reply_err(req, err);
    } else {
        ep.ino = ino;
        ep.attr_timeout = ep.entry_timeout = 10.0;
        copy_statbuf(ep.attr, statbuf);
        fuse_reply_entry(req, &ep);
        if (secfs::global_vol.options.debug)
            fuse_log(
                FUSE_LOG_DEBUG, "    lookup(parent=%ld, name=%s) -> ino=%ld\n", parent, name, ino
            );
    }
}

void secfs_forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup) {
    (void)req;
    sgx_status_t sgxstat;
    int remain;

    sgxstat = ecall_fs_forget(secfs::global_vol.eid, &remain, ino, nlookup);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
    } else {
        if (secfs::global_vol.options.debug)
            fuse_log(
                FUSE_LOG_DEBUG, "    forget(ino=%ld, nlookup=%ld) -> remain=%d\n", ino, nlookup,
                remain
            );
    }
    fuse_reply_none(req);
}

void secfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    (void)fi;
    sgx_status_t sgxstat;
    int err;
    struct stat_buffer_t statbuf;
    struct stat st;

    sgxstat = ecall_fs_getattr(secfs::global_vol.eid, &err, ino, &statbuf);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else if (err) {
        fuse_reply_err(req, err);
    } else {
        copy_statbuf(st, statbuf);
        fuse_reply_attr(req, &st, 1.0);
    }
}

void secfs_setattr(
    fuse_req_t req, fuse_ino_t ino, struct stat* attr, int to_set, struct fuse_file_info* fi
) {
    (void)fi;
    sgx_status_t sgxstat;
    int err;
    struct stat_buffer_t statbuf;
    struct stat st;

    sgxstat = ecall_fs_getattr(secfs::global_vol.eid, &err, ino, &statbuf);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else if (err) {
        fuse_reply_err(req, err);
    } else {
        copy_statbuf(st, statbuf);
        if (to_set & (FUSE_SET_ATTR_ATIME | FUSE_SET_ATTR_MTIME)) {
            struct timespec tv[2];

            st.st_atim.tv_sec = 0;
            st.st_mtim.tv_sec = 0;
            st.st_atim.tv_nsec = UTIME_OMIT;
            st.st_mtim.tv_nsec = UTIME_OMIT;

            if (to_set & FUSE_SET_ATTR_ATIME_NOW)
                st.st_atim.tv_nsec = UTIME_NOW;
            else if (to_set & FUSE_SET_ATTR_ATIME)
                st.st_atim = attr->st_atim;

            if (to_set & FUSE_SET_ATTR_MTIME_NOW)
                st.st_mtim.tv_nsec = UTIME_NOW;
            else if (to_set & FUSE_SET_ATTR_MTIME)
                st.st_mtim = attr->st_mtim;
        }
        fuse_reply_attr(req, &st, 1.0);
    }
}

void secfs_mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode) {
    struct fuse_entry_param ep;
    sgx_status_t sgxstat;
    int err;
    struct stat_buffer_t statbuf;
    fuse_ino_t ino;

    sgxstat = ecall_fs_mkdir(secfs::global_vol.eid, &err, parent, name, mode, &ino, &statbuf);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else if (err) {
        fuse_reply_err(req, err);
    } else {
        ep.ino = ino;
        ep.attr_timeout = ep.entry_timeout = 1.0;
        copy_statbuf(ep.attr, statbuf);
        fuse_reply_entry(req, &ep);
    }
}

void secfs_unlink(fuse_req_t req, fuse_ino_t parent, const char* name) {
    sgx_status_t sgxstat;
    int err;

    sgxstat = ecall_fs_unlink(secfs::global_vol.eid, &err, parent, name);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else {
        fuse_reply_err(req, err);
    }
}

void secfs_rmdir(fuse_req_t req, fuse_ino_t parent, const char* name) {
    sgx_status_t sgxstat;
    int err;

    sgxstat = ecall_fs_rmdir(secfs::global_vol.eid, &err, parent, name);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else {
        fuse_reply_err(req, err);
    }
}

void secfs_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    sgx_status_t sgxstat;
    int err;

    sgxstat = ecall_fs_open(secfs::global_vol.eid, &err, ino, (open_flag_t)fi->flags);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else if (err) {
        fuse_reply_err(req, err);
    } else {
        fuse_reply_open(req, fi);
    }
}

void secfs_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info* fi) {
    (void)fi;
    sgx_status_t sgxstat;
    int err;
    if (secfs::global_vol.options.debug)
        fuse_log(FUSE_LOG_DEBUG, "read(ino=%ld, size=%ld, off=%ld)\n", ino, size, off);

    char* buf = static_cast<char*>(malloc(size));
    size_t rsize = size;

    sgxstat = ecall_fs_read(secfs::global_vol.eid, &err, ino, buf, off, &rsize);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else if (err) {
        fuse_reply_err(req, err);
    } else {
        fuse_reply_buf(req, buf, rsize);
    }

    free(buf);
}

std::unordered_map<fuse_ino_t, std::vector<char>> file_cache;

void secfs_write(
    fuse_req_t req, fuse_ino_t ino, const char* buf, size_t size, off_t off,
    struct fuse_file_info* fi
) {
    (void)fi;
    sgx_status_t sgxstat;
    int err;
    if (secfs::global_vol.options.debug)
        fuse_log(FUSE_LOG_DEBUG, "write(ino=%ld, size=%ld, off=%ld)\n", ino, size, off);

    // auto iter = file_cache.emplace(ino, std::vector<char>(0)).first;
    // std::vector<char>& cache = file_cache.find(ino)->second;
    // if (cache.size() < size + off) {
    //     cache.resize(size + off);
    //     memcpy(cache.data() + off, buf, size);
    // }
    // fuse_reply_write(req, size);
    // return;

    size_t wsize = size;

    sgxstat = ecall_fs_write(secfs::global_vol.eid, &err, ino, buf, off, &wsize);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else if (err) {
        fuse_reply_err(req, err);
    } else {
        fuse_reply_write(req, wsize);
    }
}

void secfs_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    (void)fi;
    sgx_status_t sgxstat;
    int err;

    // auto iter = file_cache.find(ino);
    // if (iter != file_cache.end()) {
    //     std::vector<char>& cache = iter->second;
    //     size_t cache_len = 32 * 1024 * 1024;
    //     for(size_t off = 0; off < cache.size(); off += cache_len){
    //         size_t size = std::min(cache.size() - off, cache_len);
    //         sgxstat = ecall_fs_write(secfs::global_vol.eid, &err, ino, cache.data() + off, off, &size);
    //         if (sgxstat != SGX_SUCCESS) {
    //             std::cerr << enclave_err_msg(sgxstat) << std::endl;
    //             fuse_reply_err(req, ENOENT);
    //             return;
    //         }
    //     }
    // }

    sgxstat = ecall_fs_flush(secfs::global_vol.eid, &err, ino);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else {
        fuse_reply_err(req, err);
    }
}

void secfs_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    sgx_status_t sgxstat;
    int err;
    size_t count;

    sgxstat = ecall_fs_get_dirent_size(secfs::global_vol.eid, &err, ino, &count);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else if (err) {
        fuse_reply_err(req, err);
    }

    struct dirent_t* dirp = (struct dirent_t*)malloc(sizeof(struct dirent_t) * count);

    sgxstat = ecall_fs_get_dirent(secfs::global_vol.eid, &err, ino, dirp, count);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else if (err) {
        fuse_reply_err(req, err);
    } else {
        std::vector<dirent_t>* dirvec = new std::vector<dirent_t>(dirp, dirp + count);
        fi->fh = (uint64_t)dirvec;

        if (secfs::global_vol.options.debug)
            fuse_log(FUSE_LOG_DEBUG, "opendir(ino=%ld get %ld entries)\n", ino, dirvec->size());
        fuse_reply_open(req, fi);
    }
    free(dirp);
}

void secfs_readdir(
    fuse_req_t req, fuse_ino_t ino, size_t size, off_t offset, struct fuse_file_info* fi
) {
    if (secfs::global_vol.options.debug)
        fuse_log(FUSE_LOG_DEBUG, "readdir(ino=%ld, size=%ld, offset=%ld)\n", ino, size, offset);
    size_t rem = size;
    size_t entsize;
    char* buf = (char*)calloc(1, size);
    char* p = buf;
    struct stat st;
    std::vector<dirent_t>& dirent = *reinterpret_cast<std::vector<dirent_t>*>(fi->fh);
    for (size_t i = offset; i < dirent.size(); i++) {
        memset(&st, 0, sizeof(struct stat));
        dirent_t& dent = dirent[i];
        st.st_mode = dent.type << 12;
        st.st_ino = dent.ino;
        entsize = fuse_add_direntry(req, p, rem, dent.name, &st, i + 1);
        if (entsize > rem) {
            break;
        }
        p += entsize;
        rem -= entsize;
        if (secfs::global_vol.options.debug)
            fuse_log(
                FUSE_LOG_DEBUG, "    add dent %s ino=%4ld type=%03o entisze=%ld remain=%ld\n",
                dent.name, dent.ino, dent.type, entsize, rem
            );
    }

    fuse_reply_buf(req, buf, size - rem);
    free(buf);
}

void secfs_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    (void)ino;
    delete reinterpret_cast<std::vector<dirent_t>*>(fi->fh);
    fuse_reply_err(req, 0);
}

void secfs_access(fuse_req_t req, fuse_ino_t ino, int mask) {
    sgx_status_t sgxstat;
    int err;
    sgxstat = ecall_fs_access(secfs::global_vol.eid, &err, ino, mask);

    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else {
        fuse_reply_err(req, err);
    }
}

void secfs_create(
    fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode, struct fuse_file_info* fi
) {
    struct fuse_entry_param ep;
    sgx_status_t sgxstat;
    int err;
    struct stat_buffer_t statbuf;
    fuse_ino_t ino;

    sgxstat = ecall_fs_create(secfs::global_vol.eid, &err, parent, name, mode, &ino, &statbuf);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else if (err) {
        fuse_reply_err(req, err);
    } else {
        ep.ino = ino;
        ep.attr_timeout = ep.entry_timeout = 1.0;
        copy_statbuf(ep.attr, statbuf);
        fuse_reply_create(req, &ep, fi);
    }
}

void secfs_fallocate(
    fuse_req_t req, fuse_ino_t ino, int mode, off_t offset, off_t length, struct fuse_file_info* fi
){
    (void)req;
    (void)fi;
    sgx_status_t sgxstat;
    int err;

    if (secfs::global_vol.options.debug)
        fuse_log(FUSE_LOG_DEBUG, "fallocate(ino=%ld, mode=%d, offset=%ld, length=%ld)\n", ino, mode, offset, length);

    sgxstat = ecall_fs_fallocate(secfs::global_vol.eid, &err, ino, mode, offset, length);

    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    } else {
        fuse_reply_err(req, err);
    }
}
