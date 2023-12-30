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

        bool set_pubkey(const mbedtls_ecp_keypair* key);
        void dump(userinfo_t* userinfo) const;

        bool is_owner;
        mbedtls_ecp_keypair pubkey;

        static mbedtls_ecp_group_id grp_id;
    };

    using Metadata::Metadata;
    ~Usertable() override = default;

    bool load(const void* buf, size_t bsize) override;
    size_t dump(void* buf, size_t bsize) const override;

    std::unordered_map<uid_t, Userinfo> usermap;
};
