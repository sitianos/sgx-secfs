#include "volume.hpp"

std::unordered_map<ino_t, std::shared_ptr<Inode>> inode_map;
std::shared_ptr<Superinfo> superinfo;
const char* volkey_aad = "volume key";
uint8_t volkey[VOLKEYSIZE];
