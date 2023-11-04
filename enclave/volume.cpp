#include "volume.hpp"

std::unordered_map<ino_t, std::shared_ptr<Inode>> inode_map;
ino_t max_ino = 2;
