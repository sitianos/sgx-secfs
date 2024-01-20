#pragma once
#include "bridge.hpp"
#include "dirnode.hpp"
#include "filenode.hpp"
#include "metadata.hpp"
#include "superinfo.hpp"
#include "uuid.hpp"

#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

extern std::unordered_map<ino_t, std::shared_ptr<Inode>> inode_map;
extern std::shared_ptr<Superinfo> superinfo;
extern const char* volkey_aad;
extern uint8_t volkey[VOLKEYSIZE];

class ChunkStore;

class ChunkCache {
    friend class ChunkStore;

  private:
    UUID _uuid;

  public:
    std::vector<uint8_t> data;
    bool modified;
    std::shared_ptr<Filenode> filenode;
    size_t chunk_idx;
    ChunkCache() = default;
    ChunkCache(ChunkCache&& cache) = default;
    ChunkCache(const ChunkCache& cache) = delete;
    ChunkCache& operator=(const ChunkCache& cache) = delete;
    inline size_t size() {
        return data.size();
    }
    inline Chunk& chunk() {
        return filenode->chunks[chunk_idx];
    }
    // inline uint8_t* data() {
    //     return data.data();
    // }
};

class ChunkStore : public std::list<ChunkCache> {
  private:
    std::unordered_map<UUID, iterator> _map;
    std::mutex mutex;

  public:
    void push_back(ChunkCache&& cache);
    void pop_front();
    ChunkCache get_pop_front();
    iterator insert(iterator iter, ChunkCache&& cache);
    iterator erase(iterator iter);

    iterator findbykey(const UUID& uuid);
    iterator erasebykey(const UUID& uuid);
};

extern ChunkStore local_cache;
