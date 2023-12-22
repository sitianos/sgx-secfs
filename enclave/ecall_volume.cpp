#include "dirnode.hpp"
#include "enclave_t.h"
#include "filenode.hpp"
#include "metadata.hpp"
#include "storage.hpp"
#include "superinfo.hpp"
#include "usertable.hpp"
#include "uuid.hpp"
#include "volume.hpp"

#include <mbedtls/ecp.h>
#include <memory>

int ecall_create_volume(mbedtls_ecp_keypair* pubkey, char* sp_uuid_out) {
    UUID sp_uuid = UUID::gen_rand();
    UUID ut_uuid = UUID::gen_rand();
    UUID rt_uuid = UUID::gen_rand();
    std::unique_ptr<Superinfo> sp_info(Metadata::create<Superinfo>(sp_uuid));
    std::unique_ptr<Usertable> user_tb(Metadata::create<Usertable>(ut_uuid));
    std::unique_ptr<Dirnode> root_dn(Metadata::create<Dirnode>(rt_uuid));

    // Usertable::Userinfo owner;
    Usertable::Userinfo& owner = user_tb->usermap[1] = Usertable::Userinfo();

    owner.is_owner = true;
    if (!owner.set_pubkey(pubkey)) {
        printf("failed to load public key\n");
        return 1;
    }

    root_dn->ino = 1;
    root_dn->name = "";

    sp_info->user_table = ut_uuid;
    sp_info->root_dirnode = rt_uuid;
    sp_info->max_ino = 2;

    if (!save_metadata(user_tb.get())) {
        printf("failed to save usertable\n");
        return 1;
    }

    if (!save_metadata(root_dn.get())) {
        printf("failed to save root dirnode\n");
        return 1;
    }
    if (!save_metadata(sp_info.get())) {
        printf("failed to save superinfo\n");
        return 1;
    }
    sp_uuid.unparse(sp_uuid_out);
    return 0;
}

int ecall_mount_volume(size_t uid, const mbedtls_ecp_keypair* key, uuid_t uuid) {
    UUID sp_uuid(uuid);

    Superinfo* sp_p = load_metadata<Superinfo>(sp_uuid);

    if(sp_p == nullptr) {
        printf("failed to load superinfo\n");
        return 1;
    }

    superinfo = std::shared_ptr<Superinfo>(sp_p);

    Dirnode* root_dir_p = load_metadata<Dirnode>(superinfo->root_dirnode);

    if(root_dir_p == nullptr) {
        printf("failed to load root dirnode\n");
        return 1;
    }
    if(root_dir_p->ino != 1) {
        printf("inode number of root directory is not 1\n");
        return 1;
    }

    inode_map[1] = std::shared_ptr<Dirnode>(root_dir_p);

    return 0;
}
