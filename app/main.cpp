#include "enclave.hpp"
#include "enclave_u.h"
#include "fuse_operations.hpp"
#include "fuse_utils.hpp"
#include "volume.hpp"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fuse_lowlevel.h>

static void secfs_help() {
    printf("    -o config=file          JSON file for volume configuration\n"
           "    -o writeback           Enable writeback\n"
           "    -o no_writeback        Disable write back\n");
}

static const struct fuse_lowlevel_ops secfs_oper = {
    .init = secfs_init,
    .lookup = secfs_lookup,
    .forget = secfs_forget,
    .getattr = secfs_getattr,
    .setattr = secfs_setattr,
    .mkdir = secfs_mkdir,
    .unlink = secfs_unlink,
    .rmdir = secfs_rmdir,
    .open = secfs_open,
    .read = secfs_read,
    .write = secfs_write,
    .flush = secfs_flush,
    .opendir = secfs_opendir,
    .readdir = secfs_readdir,
    .releasedir = secfs_releasedir,
    .access = secfs_access,
    .create = secfs_create
};

struct secfs_options {
    char* volume_config;
    int writeback;
};

static const struct fuse_opt option_spec[] = {
    {"config=%s", offsetof(secfs_options, volume_config), 0},
    {"writeback", offsetof(secfs_options, writeback), 1},
    {"no_writeback", offsetof(secfs_options, writeback), 0},
    FUSE_OPT_END,
};

int main(int argc, char** argv, char** envp) {
    (void)envp;
    FuseSession session(argc, argv);
    secfs_options options = {.volume_config = nullptr, .writeback = 0};
    using secfs::global_vol;

    if (session.parse_cmdline() != 0) {
        std::cerr << "failed to parse command line" << std::endl;
        return 1;
    }
    if (session.opts.show_help) {
        printf("usage: %s [options] <mountpoint>\n\n", argv[0]);
        fuse_cmdline_help();
        fuse_lowlevel_help();
        secfs_help();
        return 0;
    }
    if (session.opts.show_version) {
        printf("FUSE library version %s\n", fuse_pkgversion());
        fuse_lowlevel_version();
        return 0;
    }
    if (session.opts.mountpoint == nullptr) {
        printf("usage: %s [options] <mountpoint>\n", argv[0]);
        printf("       %s --help\n", argv[0]);
        return 1;
    }
    if (session.opt_parse(&options, option_spec) != 0) {
        std::cerr << "failed to parse options" << std::endl;
        return 1;
    }
    if (options.volume_config == nullptr) {
        std::cerr << "configuration file is not given" << std::endl;
        return 1;
    }
    if (!global_vol.load_config(options.volume_config)) {
        std::cerr << "volume is not loaded" << std::endl;
        return 1;
    }

    global_vol.options.debug = session.opts.debug;
    global_vol.options.writeback = options.writeback;
    free(options.volume_config);
    if (!global_vol.mount_volume()) {
        std::cerr << "failed to mount volume" << std::endl;
        return 1;
    }

    if (session.session_new(&secfs_oper, sizeof(secfs_oper), nullptr) == nullptr) {
        std::cerr << "failed to start session" << std::endl;
        return 1;
    }
    if (session.set_signal_handlers() != 0) {
        std::cerr << "failed to set signal handler" << std::endl;
        return 1;
    }
    if (session.mount() != 0) {
        std::cerr << "failed to mount filesystem" << std::endl;
        return 1;
    }
    if (session.daemonize() != 0) {
        std::cerr << "failed to daemonize filesystem" << std::endl;
        return 1;
    }
    if (session.enter_loop() != 0) {
        std::cerr << "failed to enter loop" << std::endl;
        return 1;
    }
    return 0;
}
