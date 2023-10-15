#include <iostream>

#include "nlohmann/json.hpp"

using json = nlohmann::json;

namespace secfs {

    class StorageAPI {
       public:
        virtual int load_config(json config) = 0;
        virtual int init() = 0;

        // virtual int create_file(const char *filename) = 0;

        // virtual int create_dir(const char **path);

        // virtual int list_files();

        // virtual int get_stat(const char *filename);

        virtual ssize_t set_content(const char *filename, const char *buf, size_t size) = 0;
        virtual ssize_t get_content(const char *filename, char *buf, size_t size) = 0;

        virtual int remove_file(const char *filename) = 0;
    };

}  // namespace secfs
