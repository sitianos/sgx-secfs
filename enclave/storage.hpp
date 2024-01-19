#include "enclave_t.h"
#include "filenode.hpp"
#include "metadata.hpp"
#include "volume.hpp"

void hexdump(const void* bytes, size_t len, char* out);

int print_hex(const void* bytes, size_t len);

int printf(const char* fmt, ...);

int print_sgx_err(sgx_status_t sgxstat);

bool load_metadata(Metadata& metadata);

bool save_metadata(const Metadata& metadata);

bool remove_metadata(const UUID& uuid);

ChunkStore::iterator load_chunk(ChunkStore& store, std::shared_ptr<Filenode> fn, size_t chunk_idx);

ChunkStore::iterator save_chunk(ChunkStore& store, std::shared_ptr<Filenode> fn, size_t chunk_idx);

bool flush_chunk(ChunkStore& store, Chunk& chunk);

bool remove_chunk(const UUID& uuid);
