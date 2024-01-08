#include "metadata.hpp"

Metadata::Metadata(const UUID& _uuid) : uuid(_uuid) {
}

Inode::Inode(const UUID& _uuid) : Metadata(_uuid), refcount(0) {
}
