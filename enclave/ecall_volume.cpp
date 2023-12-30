#include "dirnode.hpp"
#include "enclave_t.h"
#include "filenode.hpp"
#include "metadata.hpp"
#include "storage.hpp"
#include "superinfo.hpp"
#include "usertable.hpp"
#include "uuid.hpp"
#include "volume.hpp"

#include <cstring>
#include <mbedtls/ecp.h>
#include <memory>
#include <sgx_trts.h>
#include <sgx_tseal.h>

size_t ecall_calc_volkey_size() {
    return sgx_calc_sealed_data_size(strlen(volkey_aad), VOLKEYSIZE);
}

int ecall_create_volume(
    mbedtls_ecp_keypair* pubkey, uuid_t sp_uuid_out, unsigned char* sealed_volkey,
    size_t volkey_size
) {
    UUID sp_uuid = UUID::gen_rand();
    UUID ut_uuid = UUID::gen_rand();
    UUID rt_uuid = UUID::gen_rand();
    std::unique_ptr<Superinfo> sp_info = std::make_unique<Superinfo>(sp_uuid);
    std::unique_ptr<Usertable> user_tb = std::make_unique<Usertable>(ut_uuid);
    std::unique_ptr<Dirnode> root_dn = std::make_unique<Dirnode>(rt_uuid);

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

    sgx_status_t sgxstat;
    sgx_read_rand(volkey, sizeof(volkey));

    print_hex(volkey, sizeof(volkey));

    size_t key_size = sgx_calc_sealed_data_size(strlen(volkey_aad), VOLKEYSIZE);
    if (volkey_size < key_size) {
        printf("given key size is lower than required key size\n");
        return 1;
    }

    sgxstat = sgx_seal_data(
        strlen(volkey_aad), (unsigned char*)volkey_aad, sizeof(volkey), volkey, volkey_size,
        (sgx_sealed_data_t*)sealed_volkey
    );

    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        memset(sealed_volkey, 0, volkey_size);
        return 1;
    }

    if (!save_metadata(*user_tb)) {
        printf("failed to save usertable\n");
        return 1;
    }

    if (!save_metadata(*root_dn)) {
        printf("failed to save root dirnode\n");
        return 1;
    }
    if (!save_metadata(*sp_info)) {
        printf("failed to save superinfo\n");
        return 1;
    }
    sp_uuid.dump(sp_uuid_out);
    return 0;
}

int ecall_mount_volume(
    size_t uid, const mbedtls_ecp_keypair* key, uuid_t sp_uuid_in,
    const unsigned char* sealed_volkey, size_t volkey_size
) {
    sgx_status_t sgxstat;
    uint32_t mac_text_len = sgx_get_add_mac_txt_len((const sgx_sealed_data_t*)sealed_volkey);
    uint32_t decrypt_data_len = sgx_get_encrypt_txt_len((const sgx_sealed_data_t*)sealed_volkey);
    if (mac_text_len != strlen(volkey_aad) || decrypt_data_len != VOLKEYSIZE) {
        printf("length of aad or volume key does not match\n");
        return 1;
    }
    if (mac_text_len > volkey_size || decrypt_data_len > volkey_size) {
        printf("the size of given sealed volume key is bigger than expected\n");
        return 1;
    }

    unsigned char dec_volkey_aad[mac_text_len];

    sgxstat = sgx_unseal_data(
        (const sgx_sealed_data_t*)sealed_volkey, dec_volkey_aad, &mac_text_len, volkey,
        &decrypt_data_len
    );
    if (sgxstat != SGX_SUCCESS) {
        printf("SGX Error in %s(): (0x%4x) ", __func__);
        print_sgx_err(sgxstat);
        return 1;
    }

    print_hex(volkey, sizeof(volkey));

    UUID sp_uuid(sp_uuid_in);
    superinfo = std::make_shared<Superinfo>(sp_uuid);

    if (!load_metadata(*superinfo)) {
        printf("failed to load superinfo\n");
        return 1;
    }

    std::shared_ptr<Dirnode> root_dir_p = std::make_shared<Dirnode>(superinfo->root_dirnode);

    if (!load_metadata(*root_dir_p)) {
        printf("failed to load root dirnode\n");
        return 1;
    }
    if (root_dir_p->ino != 1) {
        printf("inode number of root directory is not 1\n");
        return 1;
    }
    inode_map[1] = root_dir_p;

    return 0;
}
