#pragma once

#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <fstream>

#include "contracts.hpp"

using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;

using mvo = fc::mutable_variant_object;

namespace trail {

    namespace  testing {

        class trail_tester : public tester {
        public:
            enum class setup_mode {
                none,
                basic,
                full
            };

            trail_tester(setup_mode mode = setup_mode::full) {

            }
        };

    }
    
}