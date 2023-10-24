#define FUSE_USE_VERSION 34
#include "volume.hpp"

#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <fuse_lowlevel.h>

static void secfs_help() {
    printf("    --config file          JSON file for volume configuration\n");
}

static const struct fuse_lowlevel_ops secfs_oper = {};

struct secfs_options {
    const char* volume_config;
};

#define OPTION(t, p)                                                                               \
    { t, offsetof(struct secfs_options, p), 1 }

static const struct fuse_opt option_spec[] = {OPTION("--config %s", volume_config), FUSE_OPT_END};

int main(int argc, char** argv, char** envp) {
    struct fuse_args args = FUSE_ARGS_INIT(argc, argv);
    struct fuse_session* se;
    struct fuse_cmdline_opts opts;
    struct fuse_loop_config config;
    struct secfs_options options = {};
    secfs::Volume vol;
    int ret = -1;

    if (fuse_opt_parse(&args, &options, option_spec, NULL) == -1)
        return 1;

    puts(options.volume_config);
    fflush(stdout);

    if (fuse_parse_cmdline(&args, &opts) != 0)
        return 1;
    if (opts.show_help) {
        printf("usage: %s [options] <mountpoint>\n\n", argv[0]);
        fuse_cmdline_help();
        fuse_lowlevel_help();
        secfs_help();
        ret = 0;
        goto err_out1;
    } else if (opts.show_version) {
        printf("FUSE library version %s\n", fuse_pkgversion());
        fuse_lowlevel_version();
        ret = 0;
        goto err_out1;
    }

    if (opts.mountpoint == NULL) {
        printf("usage: %s [options] <mountpoint>\n", argv[0]);
        printf("       %s --help\n", argv[0]);
        ret = 1;
        goto err_out1;
    }

    vol.load_config(options.volume_config);

    if(!vol.loaded()){
        std::cerr << "volume is not loaded" << std::endl;
    }
    goto err_out1;

    se = fuse_session_new(&args, &secfs_oper, sizeof(secfs_oper), NULL);

    if (se == NULL)
        goto err_out1;

    if (fuse_set_signal_handlers(se) != 0)
        goto err_out2;

    if (fuse_session_mount(se, opts.mountpoint) != 0)
        goto err_out3;

    fuse_daemonize(opts.foreground);

    if (opts.singlethread)
        ret = fuse_session_loop(se);
    else {
        config.clone_fd = opts.clone_fd;
        config.max_idle_threads = opts.max_idle_threads;
        ret = fuse_session_loop_mt(se, &config);
    }

    fuse_session_unmount(se);
err_out3:
    delete &vol;
    fuse_remove_signal_handlers(se);
err_out2:
    fuse_session_destroy(se);
err_out1:
    free(opts.mountpoint);
    fuse_opt_free_args(&args);

    return ret ? 1 : 0;
}
