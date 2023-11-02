#include "storage.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

namespace secfs {

class LocalStorage : public StorageAPI {
  private:
    fs::path base_dir;

  public:
    int init() override;
    int destroy() override;

    ssize_t set_content(const char* filename, const void* buf, size_t size) override;
    ssize_t get_content(const char* filename, void* buf, size_t size) override;
    ssize_t get_size(const char* filename) override;

    int remove_file(const char* filename) override;

    static LocalStorage load_config(const json& config);
};

} // namespace secfs
