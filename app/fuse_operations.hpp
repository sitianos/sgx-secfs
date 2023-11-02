#define FUSE_USE_VERSION 34

#include <fuse_lowlevel.h>

extern "C" {

void secfs_getattr(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi);

void secfs_mkdir(fuse_req_t req, fuse_ino_t parent, const char *name, mode_t mode);

void secfs_rmdir(fuse_req_t req, fuse_ino_t parent, const char *name);

void secfs_lookup(fuse_req_t req, fuse_ino_t parent, const char* name);

void secfs_opendir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi);

void secfs_readdir(fuse_req_t req, fuse_ino_t ino, size_t size, off_t offset,
                   struct fuse_file_info* fi);

void secfs_releasedir(fuse_req_t req, fuse_ino_t ino, struct fuse_file_info* fi);

}
