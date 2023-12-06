#include "metadata.hpp"

size_t Metadata::dump_to_buffer(void* buf, size_t size) const {
    if (buf == nullptr) {
        return dump(nullptr, 0);
    } else {
        return dump(buf, size);
    }
}
