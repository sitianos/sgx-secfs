#pragma once

#include <stdint.h>
#include <sys/types.h>
#define MAX_PATH_LEN 256
#define CHUNKSIZE 4096

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif

typedef uint64_t fuse_ino_t;
typedef uint32_t user_id_t;
typedef uint64_t ino_t;
typedef uint32_t mode_t;
typedef uint64_t nlink_t;

typedef unsigned char uuid_t[16];
typedef unsigned char hash_t[32];

typedef unsigned char pubkey_t[256];

enum open_flag_t {
    OF_RDONLY = 00000000,
    OF_WRONLY = 00000001,
    OF_RDWR = 00000002,
    OF_TRUNC = 00001000
};

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
    ino_t max_ino;
};

struct chunk_t {
    uuid_t uuid;
};

struct filenode_buffer_t {
    ino_t ino;
    size_t size;
    size_t entnum;
    struct chunk_t entry[];
};

enum dirent_type_t {
    T_DT_DIR = 004,
    T_DT_REG = 010,
    T_DT_LNK = 012,
};

struct dirent_t {
    char name[MAX_PATH_LEN];
    uuid_t uuid;
    ino_t ino;
    enum dirent_type_t type;
};

struct dirnode_buffer_t {
    ino_t ino;
    char name[MAX_PATH_LEN];
    size_t entnum;
    struct dirent_t entry[];
};

struct userinfo_t {
    int is_owner;
    user_id_t uid;
    size_t keysize;
    pubkey_t pubkey;
};

struct usertable_buffer_t {
    size_t entnum;
    struct userinfo_t entry[];
};

#ifdef __cplusplus
}
#endif
