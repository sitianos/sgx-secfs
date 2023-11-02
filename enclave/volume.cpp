#include "volume.hpp"

std::unordered_map<ino_t, std::shared_ptr<Inode>> dirnode_map;
ino_t dirnode_ino = 2;
