#include "metadata.hpp"
#include "dirnode.hpp"
#include "superinfo.hpp"
#include "bridge.hpp"

#include <unordered_map>
#include <memory>

extern std::unordered_map<ino_t, std::shared_ptr<Inode>> inode_map;
extern ino_t max_ino;
