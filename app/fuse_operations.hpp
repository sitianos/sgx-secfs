#define FUSE_USE_VERSION 34

#include <fuse_lowlevel.h>

extern "C" {

void secfs_init(void *userdata, struct fuse_conn_info *conn);
 
// void secfs_destroy(void *userdata);

void secfs_lookup(fuse_req_t req, fuse_ino_t parent, const char* name);

void secfs_forget(fuse_req_t req, fuse_ino_t ino, uint64_t nlookup);

void secfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi);

// void secfs_setattr(fuse_req_t req, fuse_ino_t ino, struct stat *attr, int to_set, struct fuse_file_info *fi);

// void secfs_readlink(fuse_req_t req, fuse_ino_t ino);

void secfs_mkdir(fuse_req_t req, fuse_ino_t parent, const char* name, mode_t mode);

void secfs_unlink(fuse_req_t req, fuse_ino_t parent, const char *name);

void secfs_rmdir(fuse_req_t req, fuse_ino_t parent, const char* name);

// void secfs_symlink(fuse_req_t req, const char *link, fuse_ino_t parent, const char *name);

// void secfs_rename(fuse_req_t req, fuse_ino_t parent, const char *name, fuse_ino_t newparent, const char *newname, unsigned int flags);

// void secfs_link(fuse_req_t req, fuse_ino_t ino, fuse_ino_t newparent, const char *newname);

void secfs_open(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

void secfs_read(fuse_req_t req, fuse_ino_t ino, size_t size, off_t off, struct fuse_file_info *fi);

void secfs_write(fuse_req_t req, fuse_ino_t ino, const char *buf, size_t size, off_t off, struct fuse_file_info *fi);

void secfs_flush(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

// void secfs_release(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info *fi);

void secfs_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi);

void secfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t offset,
                   struct fuse_file_info* fi);

void secfs_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi);

// void secfs_setxattr(fuse_req_t req, fuse_ino_t ino, const char *name, const char *value, size_t size, int flags);

// void secfs_getxattr(fuse_req_t req, fuse_ino_t ino, const char *name, size_t size);

// void secfs_listxattr(fuse_req_t req, fuse_ino_t ino, size_t size);

// void secfs_removexattr(fuse_req_t req, fuse_ino_t ino, const char *name);

void secfs_access(fuse_req_t req, fuse_ino_t ino, int mask);

void secfs_create(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode, struct fuse_file_info *fi);

}
