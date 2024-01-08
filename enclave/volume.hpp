#include "bridge.hpp"
#include "dirnode.hpp"
#include "metadata.hpp"
#include "superinfo.hpp"

#include <memory>
#include <unordered_map>

extern std::unordered_map<ino_t, std::shared_ptr<Inode>> inode_map;
extern std::shared_ptr<Superinfo> superinfo;
extern const char* volkey_aad;
extern uint8_t volkey[VOLKEYSIZE];
