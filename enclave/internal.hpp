#pragma once

#include "bridge.hpp"

#include <stdint.h>
#include <sys/types.h>

#ifdef __cplusplus
#include <cstddef>
extern "C" {
#endif

#define VOLKEYSIZE 256

typedef unsigned char hash_t[32];
typedef unsigned char pubkey_t[256];

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
