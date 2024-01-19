#pragma once

#include "bridge.hpp"

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif

#ifndef ENABLE_ENCRYPTION
#define ENABLE_ENCRYPTION (1)
#endif

#ifndef VOLKEYSIZE
#define VOLKEYSIZE (256)
#endif

#ifndef MAX_CACHE_NUM
#define MAX_CACHE_NUM (4)
#endif

#ifndef CHUNKSIZE
#define CHUNKSIZE (4096)
#endif

typedef uint8_t hash_t[32];
typedef uint8_t pubkey_t[256];
typedef uint8_t tag_t[16];
typedef uint8_t iv_t[12];

struct superinfo_buffer_t {
    uuid_t root_dirnode;
    uuid_t user_table;
    hash_t hash_root_dirnode;
    hash_t hash_user_table;
    ino_t max_ino;
};

struct chunk_t {
    uuid_t uuid;
    iv_t iv;
    tag_t tag;
};

struct filenode_buffer_t {
    ino_t ino;
    size_t size;
    size_t entnum;
    struct chunk_t entry[];
};

struct dirnode_buffer_t {
    ino_t ino;
    char name[MAX_PATH_LEN];
    size_t entnum;
    struct dirent_t entry[];
};

struct userinfo_t {
    int is_owner;
    uid_t uid;
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
