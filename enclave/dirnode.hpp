#include "metadata.hpp"

#include <string>
#include <vector>

#define MAX_PATH_LEN 256

class Dirnode : public Metadata {
  private:
  public:
    enum Type {
        T_DIR,
        T_REG,
        T_LINK,
    };
    struct dirent_t {
        char name[MAX_PATH_LEN];
        Type type;
        uuid_t uuid;
    };
    class Dirent {
      public:
        Dirent();
        Dirent(const dirent_t& dent);
        std::string name;
        Type type;
        UUID uuid;
        void dump(dirent_t& dent);
    };
    struct Buffer {
        char name[MAX_PATH_LEN];
        unsigned int entnum;
        struct dirent_t entry[];
    };
    std::string name;
    std::vector<Dirent> dirent;
    size_t dump_to_buffer(Buffer*& buf);
    static Dirnode load_from_buffer(const Buffer& buf);
};
