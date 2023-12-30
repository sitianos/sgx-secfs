#pragma once

#ifdef __cplusplus
#include <cstddef>
#include <cstdint>
extern "C" {
#else
#include <stdint.h>
#endif

#define MAX_PATH_LEN 256
#define CHUNKSIZE 4096

typedef uint64_t fuse_ino_t;
typedef uint32_t uid_t;
typedef uint64_t ino_t;
typedef uint32_t mode_t;
typedef uint64_t nlink_t;
typedef int64_t off_t;
typedef uint8_t uuid_t[16];

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
    size_t size;
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

#ifdef __cplusplus
}
#endif
