#include "metadata.hpp"

#include "dirnode.hpp"
#include "filenode.hpp"
#include "superinfo.hpp"


template <>
Dirnode* Metadata::create<Dirnode>(const UUID& uuid, const void* buf, size_t size) {
    Dirnode* ret = nullptr;
    if(buf) {
        ret = new Dirnode(*static_cast<const dirnode_buffer_t*>(buf));
    } else {
        ret = new Dirnode();
    }
    ret->type = Metadata::M_Dirnode;
    ret->uuid = uuid;
    return ret;
}

template <>
Filenode* Metadata::create<Filenode>(const UUID& uuid, const void* buf, size_t size) {
    Filenode* ret = nullptr;
    if(buf) {
        ret = new Filenode(*static_cast<const filenode_buffer_t*>(buf));
    } else {
        ret = new Filenode();
    }
    ret->type = Metadata::M_Filenode;
    ret->uuid = uuid;
    return ret;
}

size_t Metadata::dump_to_buffer(void* buf, size_t size) const {
    if (buf == nullptr) {
        return dump(nullptr, 0);
    } else {
        return dump(buf, size);
    }
}
