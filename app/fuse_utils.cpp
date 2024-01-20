#include "fuse_utils.hpp"

#include <cstdlib>

FuseSession::FuseSession(int argc, char** argv)
    : args(FUSE_ARGS_INIT(argc, argv)), se(nullptr), signaled(false), mounted(false) {
}

int FuseSession::parse_cmdline() {
    return fuse_parse_cmdline(&args, &opts);
}

int FuseSession::opt_parse(void* data, const fuse_opt* opts_spec) {
    return fuse_opt_parse(&args, data, opts_spec, nullptr);
}

fuse_session*
FuseSession::session_new(const fuse_lowlevel_ops* op, size_t op_size, void* userdata) {
    return se = fuse_session_new(&args, op, op_size, userdata);
}

int FuseSession::set_signal_handlers() {
    int ret = fuse_set_signal_handlers(se);
    if (ret == 0)
        signaled = true;
    return ret;
}

int FuseSession::mount() {
    int ret = fuse_session_mount(se, opts.mountpoint);
    if (ret == 0)
        mounted = true;
    return ret;
}

int FuseSession::daemonize() {
    return fuse_daemonize(opts.foreground);
}

int FuseSession::enter_loop() {
    int ret;
    if (opts.singlethread)
        ret = fuse_session_loop(se);
    else {
        config.clone_fd = opts.clone_fd;
        config.max_idle_threads = opts.max_idle_threads;
        ret = fuse_session_loop_mt(se, &config);
    }
    return ret;
}

FuseSession::~FuseSession() {
    if (mounted)
        fuse_session_unmount(se);
    if (signaled)
        fuse_remove_signal_handlers(se);
    if (se)
        fuse_session_destroy(se);
    free(opts.mountpoint);
    fuse_opt_free_args(&args);
}
