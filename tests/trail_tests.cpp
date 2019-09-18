#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <fc/variant_object.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <iostream>

#include <Runtime/Runtime.h>
#include <iomanip>

#include "trail_tester.hpp"

using namespace eosio;
using namespace eosio::testing;
using namespace eosio::chain;
using namespace fc;
using namespace std;
using namespace trail::testing;
using mvo = fc::mutable_variant_object;

BOOST_AUTO_TEST_SUITE(trail_tests)

    BOOST_FIXTURE_TEST_CASE( literally_anything, trail_tester ) try {
        int x = 0;
        BOOST_REQUIRE(true);
    } FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()

