#include "volume.hpp"

std::unordered_map<ino_t, std::shared_ptr<Inode>> inode_map;
std::shared_ptr<Superinfo> superinfo;
ino_t max_ino = 2;
