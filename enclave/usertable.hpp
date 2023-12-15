#pragma once

#include "metadata.hpp"

#include <mbedtls/ecp.h>
#include <unordered_map>

class Usertable : public Metadata {
  public:
    class Userinfo {
      public:
        Userinfo();
        Userinfo(const userinfo_t& userinfo);
        Userinfo(const userinfo_t* userinfo);
        ~Userinfo();
        bool is_owner;
        mbedtls_ecp_keypair pubkey;

        bool set_pubkey(const mbedtls_ecp_keypair* key);
        void dump(userinfo_t* userinfo) const;

        static mbedtls_ecp_group_id grp_id;
    };
    std::unordered_map<size_t, Userinfo> usermap;

    Usertable() = default;
    Usertable(const usertable_buffer_t* buf);
    Usertable(const void* buf);

    size_t dump(void* buf, size_t size) const override;
};
