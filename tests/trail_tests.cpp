#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <fc/variant_object.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <iostream>
#include <boost/container/map.hpp>

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

    BOOST_FIXTURE_TEST_CASE( configuration_setting, trail_tester ) try {

        string version = "v2.0.0-RC1";
        set_config(version, true);

        fc::variant config = get_config();

        //check defaults are set correct
        BOOST_REQUIRE_EQUAL(config["trail_version"], version);
        
        //fee validation
        map<name, asset> fee_map = variant_to_map<name, asset>(config["fees"]);

        validate_map(fee_map, name("archival"), asset::from_string("3.0000 TLOS"));

        validate_map(fee_map, name("ballot"), asset::from_string("30.0000 TLOS"));

        validate_map(fee_map, name("committee"), asset::from_string("10.0000 TLOS"));

        validate_map(fee_map, name("treasury"), asset::from_string("500.0000 TLOS"));

        //time validation
        map<name, uint32_t> time_map = variant_to_map<name, uint32_t>(config["times"]);

        validate_map(time_map, name("balcooldown"), uint32_t(432000));

        validate_map(time_map, name("minballength"), uint32_t(60));

        asset new_archival_fee = asset::from_string("100.0000 TST");

        //change fees
        update_fee(name("archival"), new_archival_fee);

        config = get_config();
        fee_map = variant_to_map<name, asset>(config["fees"]);

        validate_map(fee_map, name("archival"), new_archival_fee);



    } FC_LOG_AND_RETHROW()

BOOST_AUTO_TEST_SUITE_END()

