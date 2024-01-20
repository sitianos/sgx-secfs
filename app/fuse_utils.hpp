#define FUSE_USE_VERSION 34
#include <fuse_lowlevel.h>

class FuseSession {
  public:
    fuse_cmdline_opts opts;
    FuseSession(int argc, char** argv);
    ~FuseSession();
    int parse_cmdline();
    int opt_parse(void* data, const fuse_opt* opts_spec);
    fuse_session*
    session_new(const fuse_lowlevel_ops* op, size_t op_size, void* userdata = nullptr);
    int set_signal_handlers();
    int mount();
    int daemonize();
    int enter_loop();

  private:
    fuse_args args;
    fuse_session* se;
    fuse_loop_config config;
    bool signaled;
    bool mounted;
};
