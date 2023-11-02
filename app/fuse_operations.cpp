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
}

void secfs_lookup(fuse_req_t req, fuse_ino_t parent, const char* name) {
    (void)req;
    struct fuse_entry_param ep;
    fuse_ino_t ino;
    struct stat_buffer_t statbuf;
    fuse_log(FUSE_LOG_DEBUG, "lo_lookup(parent=%ld, name=%s)\n", parent, name);
    sgx_status_t sgxstat = ecall_fs_lookup(secfs::global_vol.eid, parent, name, &ino, &statbuf);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
    }
    if (ino == 0) {
        fuse_reply_err(req, ENOENT);
    } else {
        ep.ino = ino;
        ep.attr_timeout = ep.entry_timeout = 1.0;
        copy_statbuf(ep.attr, statbuf);
        fuse_reply_entry(req, &ep);
    }
}

void secfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    (void)req;
    (void)fi;
    struct stat_buffer_t statbuf;
    struct stat st;
    ecall_fs_getattr(secfs::global_vol.eid, ino, &statbuf);
    copy_statbuf(st, statbuf);
    fuse_reply_attr(req, &st, 1.0);
}

void secfs_mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode) {
    (void)req;
    struct stat_buffer_t statbuf;
    fuse_ino_t ino;
    struct fuse_entry_param ep;
    ecall_fs_mkdir(secfs::global_vol.eid, parent, name, mode, &ino, &statbuf);
    if (ino == 0) {
        fuse_reply_err(req, -1);
        return;
    }
    ep.ino = ino;
    ep.attr_timeout = ep.entry_timeout = 1.0;
    copy_statbuf(ep.attr, statbuf);
    fuse_reply_entry(req, &ep);
}

void secfs_rmdir(fuse_req_t req, fuse_ino_t parent, const char* name) {
    (void)req;
    ecall_fs_rmdir(secfs::global_vol.eid, parent, name);
    fuse_reply_err(req, 0);
}

void secfs_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    (void)req;
    size_t count = 16;
    ssize_t getcount;
    struct dirent_t* dirp = (struct dirent_t*)malloc(sizeof(struct dirent_t) * count);
    sgx_status_t sgxstat = ecall_fs_get_dirent(secfs::global_vol.eid, ino, dirp, count, &getcount);
    if (sgxstat != SGX_SUCCESS) {
        std::cerr << enclave_err_msg(sgxstat) << std::endl;
        fuse_reply_err(req, ENOENT);
        free(dirp);
        return;
    }
    if (getcount < 0) {
        fuse_reply_err(req, ENOENT);
        free(dirp);
        return;
    }
    std::vector<dirent_t>* dirvec = new std::vector<dirent_t>(dirp, dirp + getcount);
    fi->fh = (uint64_t)dirvec;

    fuse_log(FUSE_LOG_DEBUG, "opendir(ino=%ld get %ld entries)\n", ino, dirvec->size());
    fuse_reply_open(req, fi);
    free(dirp);
    return;
}

void secfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t offset,
                   struct fuse_file_info* fi) {
    (void)req;
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
        fuse_log(FUSE_LOG_DEBUG, "    add dent %s ino=%4ld type=%03o entisze=%ld remain=%ld\n",
                 dent.name, dent.ino, dent.type, entsize, rem);
    }

    fuse_reply_buf(req, buf, size - rem);
    free(buf);
}

void secfs_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi) {
    (void)req;
    delete reinterpret_cast<std::vector<dirent_t>*>(fi->fh);
}
