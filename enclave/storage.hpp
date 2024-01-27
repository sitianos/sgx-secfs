#include "enclave_t.h"
#include "filenode.hpp"
#include "metadata.hpp"
#include "volume.hpp"

void hexdump(const void* bytes, size_t len, char* out);

int print_hex(const void* bytes, size_t len);

#if ENABLE_DEBUG_PRINT
int printf(const char* fmt, ...);

int print_sgx_err(sgx_status_t sgxstat);
#else
#define printf(...) 0
#define print_sgx_err(...) 0
#endif

bool load_metadata(Metadata& metadata);

bool save_metadata(const Metadata& metadata);

bool remove_metadata(const UUID& uuid);

ChunkStore::iterator load_chunk(ChunkStore& store, std::shared_ptr<Filenode> fn, size_t chunk_idx);

ChunkStore::iterator save_chunk(ChunkStore& store, std::shared_ptr<Filenode> fn, size_t chunk_idx);

bool load_chunk(ChunkStore& store, ChunkCache& cache);

bool save_chunk(ChunkStore& store, ChunkCache&& cache);

bool flush_chunk(ChunkStore& store, Chunk& chunk);

bool remove_chunk(const UUID& uuid);
