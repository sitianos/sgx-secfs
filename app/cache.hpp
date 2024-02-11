#pragma once

#include <list>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>

class ChunkStore;

class ChunkCache {
    friend class ChunkStore;

  public:
    std::vector<uint8_t> data;
    bool modified;
    ino_t ino;
    size_t chunk_idx;
    ChunkCache() = default;
    ChunkCache(size_t size, ino_t ino, size_t idx);
    ChunkCache(ChunkCache&& cache) = default;
    ChunkCache(const ChunkCache& cache) = delete;
    ChunkCache& operator=(ChunkCache&& cache) = default;
    ChunkCache& operator=(const ChunkCache& cache) = delete;
    inline size_t size() {
        return data.size();
    }
};

class ChunkStore : public std::list<ChunkCache> {
  private:
    std::unordered_map<std::pair<ino_t, size_t>, iterator> _map;
    std::mutex mutex;

  public:
    void push_back(ChunkCache&& cache);
    // iterator push_get_back(ChunkCache&& cache);
    bool get_pop_front(ChunkCache& cache);
    // ChunkCache get_erase(iterator iter);
    // iterator insert(iterator iter, ChunkCache&& cache);
    // iterator erase(iterator iter);

    // iterator findbykey(ino_t ino, size_t idx);
    // iterator erasebykey(ino_t ino, size_t idx);
    bool get_erase_by_key(ino_t ino, size_t idx, ChunkCache& cache);
};

extern ChunkStore local_cache;
