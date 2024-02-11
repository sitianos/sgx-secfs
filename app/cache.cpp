#include "cache.hpp"

#include <cassert>

ChunkCache::ChunkCache(size_t size, ino_t ino, size_t idx)
    : data(size), ino(ino), chunk_idx(idx), modified(false) {
}

void ChunkStore::push_back(ChunkCache&& cache) {
    std::lock_guard<std::mutex> lock(mutex);
    decltype(_map)::const_iterator map_iter = _map.find(std::make_pair(cache.ino, cache.chunk_idx));
    if (map_iter != _map.end()) {
        std::list<ChunkCache>::erase(map_iter->second);
    }
    _map[std::make_pair(cache.ino, cache.chunk_idx)] =
        std::list<ChunkCache>::insert(end(), std::move(cache));
}

bool ChunkStore::get_pop_front(ChunkCache& cache) {
    std::lock_guard<std::mutex> lock(mutex);
    if (std::list<ChunkCache>::size() == 0) {
        return false;
    }
    cache = std::move(std::list<ChunkCache>::front());
    // printf("chunk %4ld poped  size=%ld\n", cache.chunk_idx, size());
    decltype(_map)::const_iterator map_iter = _map.find(std::make_pair(cache.ino, cache.chunk_idx));
    assert(map_iter->second == std::list<ChunkCache>::begin());
    _map.erase(map_iter);
    std::list<ChunkCache>::pop_front();
    return true;
}

// ChunkCache ChunkStore::get_erase(iterator iter) {
//     std::lock_guard<std::mutex> lock(mutex);
//     ChunkCache cache = std::move(*iter);
//     _map.erase(cache._uuid);
//     std::list<ChunkCache>::erase(iter);
//     return cache;
// }

// ChunkStore::iterator ChunkStore::insert(iterator iter, ChunkCache&& cache) {
//     std::lock_guard<std::mutex> lock(mutex);
//     UUID& uuid = cache.chunk().uuid;
//     decltype(_map)::iterator chunk_iter = _map.find(uuid);
//     cache._uuid = uuid;
//     if (chunk_iter != _map.end()) {
//         std::list<ChunkCache>::erase(chunk_iter->second);
//     }
//     return _map[uuid] = std::list<ChunkCache>::insert(iter, std::move(cache));
// }

// ChunkStore::iterator ChunkStore::erase(iterator iter) {
//     std::lock_guard<std::mutex> lock(mutex);
//     decltype(_map)::iterator map_iter = _map.find(iter->_uuid);
//     assert(map_iter != _map.end());
//     _map.erase(map_iter);
//     return std::list<ChunkCache>::erase(iter);
// }

// ChunkStore::iterator ChunkStore::findbykey(ino_t ino, size_t idx){
//     std::lock_guard<std::mutex> lock(mutex);
//     decltype(_map)::iterator map_iter = _map.find(uuid);
//     if (map_iter == _map.end()) {
//         return this->end();
//     }
//     assert(map_iter->second->_uuid == uuid);
//     return map_iter->second;
// }

// ChunkStore::iterator ChunkStore::erasebykey(ino_t ino, size_t idx){
//     std::lock_guard<std::mutex> lock(mutex);
//     decltype(_map)::iterator map_iter = _map.find(std::make_pair(ino, idx));
//     if (map_iter == _map.end()) {
//         return this->end();
//     }
//     assert(map_iter->second-> == uuid);
//     iterator iter = std::list<ChunkCache>::erase(map_iter->second);
//     _map.erase(map_iter);
//     return iter;
// }

bool ChunkStore::get_erase_by_key(ino_t ino, size_t idx, ChunkCache& cache) {
    std::lock_guard<std::mutex> lock(mutex);
    decltype(_map)::iterator map_iter = _map.find(std::make_pair(ino, idx));
    if (map_iter == _map.end()) {
        return false;
    }
    cache = std::move(*map_iter->second);
    std::list<ChunkCache>::erase(map_iter->second);
    _map.erase(map_iter);
    return true;
}

ChunkStore local_cache;
