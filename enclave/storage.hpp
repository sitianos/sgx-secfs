#include "enclave_t.h"
#include "filenode.hpp"
#include "metadata.hpp"

void hexdump(const void* bytes, size_t len, char* out);

int print_hex(const void* bytes, size_t len);

int printf(const char* fmt, ...);

int print_sgx_err(sgx_status_t sgxstat);

bool load_metadata(Metadata& metadata);

bool save_metadata(const Metadata& metadata);

bool remove_metadata(const UUID& uuid);

ssize_t load_chunk(Filenode::Chunk& chunk);

ssize_t save_chunk(Filenode::Chunk& chunk);

bool remove_chunk(const UUID& uuid);
