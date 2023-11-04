#pragma once

#include <stdint.h>
#include <sys/types.h>
#define MAX_PATH_LEN 256

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif

typedef uint64_t fuse_ino_t;

typedef uint64_t ino_t;
typedef uint32_t mode_t;
typedef uint64_t nlink_t;

typedef unsigned char uuid_t[16];
typedef unsigned char hash_t[32];

enum stat_mode_t {
    T_ST_DIR = 0040000,
    T_ST_REG = 0100000,
    T_ST_LNK = 0120000,
};

struct stat_buffer_t {
    ino_t ino;
    enum stat_mode_t mode;
    nlink_t nlink;
    off_t size;
};

struct superinfo_buffer_t {
    uuid_t root_dirnode;
    uuid_t user_table;
    hash_t hash_root_dirnode;
    hash_t hash_user_table;
};

struct chunk_t {
    uuid_t uuid;
};

struct filenode_buffer_t {
    ino_t ino;
    size_t size;
    size_t chunknum;
    struct chunk_t entry[];
};

enum dirent_type_t {
    T_DT_DIR = 004,
    T_DT_REG = 010,
    T_DT_LNK = 012,
};

struct dirent_t {
    ino_t ino;
    char name[MAX_PATH_LEN];
    enum dirent_type_t type;
    uuid_t uuid;
};

struct dirnode_buffer_t {
    ino_t ino;
    char name[MAX_PATH_LEN];
    size_t entnum;
    struct dirent_t entry[];
};

#ifdef __cplusplus
}
#endif
