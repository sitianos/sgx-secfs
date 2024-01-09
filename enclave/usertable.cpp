#include "usertable.hpp"

#include <cstring>

bool Usertable::load(const void* buf, size_t bsize) {
    const usertable_buffer_t* obuf = static_cast<const usertable_buffer_t*>(buf);
    size_t rqsize = sizeof(usertable_buffer_t) + sizeof(userinfo_t) * obuf->entnum;
    if (bsize != rqsize) {
        return false;
    }
    for (size_t i = 0; i < obuf->entnum; i++) {
        const userinfo_t& user = obuf->entry[i];
        if (usermap.count(user.uid) != 0) {
            return false;
        }
        try {
            usermap.emplace(user.uid, user);
        } catch (std::exception& e) {
            return false;
        }
    }
    return true;
}

size_t Usertable::dump(void* buf, size_t bsize) const {
    size_t rqsize = sizeof(usertable_buffer_t) + sizeof(userinfo_t) * usermap.size();
    if (buf == nullptr) {
        return rqsize;
    }
    if (bsize < rqsize) {
        return 0;
    }

    usertable_buffer_t* obuf = static_cast<usertable_buffer_t*>(buf);

    obuf->entnum = usermap.size();
    size_t idx = 0;
    for (const auto& [uid, userinfo] : usermap) {
        obuf->entry[idx].uid = uid;
        userinfo.dump(&obuf->entry[idx]);
    }
    return rqsize;
}

mbedtls_ecp_group_id Usertable::Userinfo::grp_id = MBEDTLS_ECP_DP_SECP384R1;

Usertable::Userinfo::Userinfo() {
    mbedtls_ecp_keypair_init(&pubkey);
    if (mbedtls_ecp_group_load(&pubkey.MBEDTLS_PRIVATE(grp), grp_id) != 0) {
        throw std::invalid_argument("mbedtls_ecp_group_load");
    }
}

Usertable::Userinfo::Userinfo(const userinfo_t& userinfo) : Userinfo() {
    is_owner = userinfo.is_owner;
    if (mbedtls_ecp_point_read_binary(
            &pubkey.MBEDTLS_PRIVATE(grp), &pubkey.MBEDTLS_PRIVATE(Q), userinfo.pubkey,
            userinfo.keysize
        ) != 0) {
        mbedtls_ecp_keypair_init(&pubkey);
        throw std::invalid_argument("mbedtls_ecp_point_read_binary");
    }
    if (mbedtls_ecp_check_pubkey(&pubkey.MBEDTLS_PRIVATE(grp), &pubkey.MBEDTLS_PRIVATE(Q)) != 0) {
        throw std::invalid_argument("mbedtls_ecp_check_pubkey");
    }
}

Usertable::Userinfo::Userinfo(const userinfo_t* userinfo) : Userinfo(*userinfo) {
}

Usertable::Userinfo::~Userinfo() {
    mbedtls_ecp_keypair_free(&pubkey);
}

bool Usertable::Userinfo::load(const userinfo_t& userinfo) {
    is_owner = userinfo.is_owner;
    if (mbedtls_ecp_point_read_binary(
            &pubkey.MBEDTLS_PRIVATE(grp), &pubkey.MBEDTLS_PRIVATE(Q), userinfo.pubkey,
            userinfo.keysize
        ) != 0) {
        mbedtls_ecp_keypair_init(&pubkey);
        return false;
    }
    if (mbedtls_ecp_check_pubkey(&pubkey.MBEDTLS_PRIVATE(grp), &pubkey.MBEDTLS_PRIVATE(Q)) != 0) {
        return false;
    }
    return true;
}

bool Usertable::Userinfo::set_pubkey(const mbedtls_ecp_keypair* key) {
    if (pubkey.MBEDTLS_PRIVATE(grp).id != key->MBEDTLS_PRIVATE(grp).id)
        return false;
    if (mbedtls_ecp_copy(&pubkey.MBEDTLS_PRIVATE(Q), &key->MBEDTLS_PRIVATE(Q)) != 0)
        return false;
    if (mbedtls_ecp_check_pubkey(&pubkey.MBEDTLS_PRIVATE(grp), &pubkey.MBEDTLS_PRIVATE(Q)) != 0)
        return false;
    return true;
}

void Usertable::Userinfo::dump(userinfo_t* userinfo) const {
    userinfo->is_owner = is_owner ? 1 : 0;
    std::memset(userinfo->pubkey, 0, sizeof(pubkey_t));
    if (mbedtls_ecp_point_write_binary(
            &pubkey.MBEDTLS_PRIVATE(grp), &pubkey.MBEDTLS_PRIVATE(Q), MBEDTLS_ECP_PF_UNCOMPRESSED,
            &userinfo->keysize, userinfo->pubkey, sizeof(pubkey_t)
        ) != 0) {
        userinfo->keysize = 0;
    }
}
