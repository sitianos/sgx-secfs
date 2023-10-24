#include "metadata.hpp"

class Superinfo : public Metadata {
  private:
    UUID root_dirnode;
    UUID user_table;
    hash_t hash_root_dirnode;
    hash_t hash_user_table;

  public:
    struct Buffer {
        uuid_t root_dirnode;
        uuid_t user_table;
        hash_t hash_root_dirnode;
        hash_t hash_user_table;
    };
    static Superinfo load_from_buffer(const Buffer& buf);
    Buffer dump_to_buffer();
};
