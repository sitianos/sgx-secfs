#include <filesystem>
#include <fstream>
#include <string>

#include "storage.hpp"

namespace fs = std::filesystem;

namespace secfs {

    class LocalStorage : public StorageAPI {
       private:
        fs::path base_dir;

       public:
        int load_config(json config);
        int init();

        ssize_t set_content(const char *filename, const char *buf, size_t size);
        ssize_t get_content(const char *filename, char *buf, size_t size);

        int remove_file(const char *filename);
    };

}  // namespace secfs
