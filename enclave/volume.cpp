#include "volume.hpp"

#include <cassert>

std::unordered_map<ino_t, std::shared_ptr<Inode>> inode_map;
std::shared_ptr<Superinfo> superinfo;
const char* volkey_aad = "volume key";
uint8_t volkey[VOLKEYSIZE];

void ChunkStore::push_back(ChunkCache&& cache) {
    UUID& uuid = cache.chunk().uuid;
    cache._uuid = uuid;
    decltype(_map)::const_iterator iter = _map.find(uuid);
    if (iter != _map.end()) {
        std::list<ChunkCache>::erase(iter->second);
    }
    _map[uuid] = std::list<ChunkCache>::insert(end(), std::move(cache));
}

void ChunkStore::pop_front() {
    ChunkCache& cache = std::list<ChunkCache>::front();
    _map.erase(cache._uuid);
    std::list<ChunkCache>::pop_front();
}

ChunkCache ChunkStore::get_pop_front() {
    std::lock_guard<std::mutex> lock(mutex);
    ChunkCache cache = std::move(std::list<ChunkCache>::front());
    assert(_map.find(cache._uuid)->second == std::list<ChunkCache>::begin());
    _map.erase(cache._uuid);
    std::list<ChunkCache>::pop_front();
    return cache;
}

ChunkStore::iterator ChunkStore::insert(iterator iter, ChunkCache&& cache) {
    UUID& uuid = cache.chunk().uuid;
    decltype(_map)::iterator chunk_iter = _map.find(uuid);
    cache._uuid = uuid;
    if (chunk_iter != _map.end()) {
        std::list<ChunkCache>::erase(chunk_iter->second);
    }
    return _map[uuid] = std::list<ChunkCache>::insert(iter, std::move(cache));
}

ChunkStore::iterator ChunkStore::erase(iterator iter) {
    decltype(_map)::iterator map_iter = _map.find(iter->_uuid);
    assert(map_iter != _map.end());
    _map.erase(map_iter);
    return std::list<ChunkCache>::erase(iter);
}

ChunkStore::iterator ChunkStore::findbykey(const UUID& uuid) {
    decltype(_map)::iterator map_iter = _map.find(uuid);
    if (map_iter == _map.end()) {
        return this->end();
    }
    assert(map_iter->second->_uuid == uuid);
    return map_iter->second;
}

ChunkStore::iterator ChunkStore::erasebykey(const UUID& uuid) {
    decltype(_map)::iterator map_iter = _map.find(uuid);
    if (map_iter == _map.end()) {
        return this->end();
    }
    assert(map_iter->second->_uuid == uuid);
    iterator iter = std::list<ChunkCache>::erase(map_iter->second);
    _map.erase(map_iter);
    return iter;
}

ChunkStore local_cache;
