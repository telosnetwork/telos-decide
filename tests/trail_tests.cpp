#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <fc/variant_object.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <iostream>
#include <boost/container/map.hpp>
#include <map>

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
        produce_blocks();

        fc::variant config = get_config();

        //check defaults are set correctly
        BOOST_REQUIRE_EQUAL(config["trail_version"], version);
        
        //fee validation
        map<name, asset> fee_map = variant_to_map<name, asset>(config["fees"]);

        validate_map(fee_map, name("archival"), asset::from_string("3.0000 TLOS"));

        validate_map(fee_map, name("ballot"), asset::from_string("30.0000 TLOS"));

        validate_map(fee_map, name("committee"), asset::from_string("10.0000 TLOS"));

        validate_map(fee_map, name("treasury"), asset::from_string("1000.0000 TLOS"));

        //time validation
        map<name, uint32_t> time_map = variant_to_map<name, uint32_t>(config["times"]);

        validate_map(time_map, name("balcooldown"), uint32_t(432000));

        validate_map(time_map, name("minballength"), uint32_t(86400));

        
        //change with improper symbol
        BOOST_REQUIRE_EXCEPTION(update_fee(name("archival"), asset::from_string("100.0000 TST")),
            eosio_assert_message_exception, eosio_assert_message_is( "fee symbol must be TLOS" ) 
        );

        //change with a zero time
        BOOST_REQUIRE_EXCEPTION(update_time(name("balcooldown"), uint32_t(0)),
            eosio_assert_message_exception, eosio_assert_message_is( "length must be a positive number" ) 
        );

        //change a fee and a time
        asset new_archival_fee = asset::from_string("100.0000 TLOS");
        uint32_t new_min_bal_length = uint32_t(1000);
        update_fee(name("archival"), new_archival_fee);
        update_time(name("minballength"), new_min_bal_length); 
        produce_blocks();

        //validate changes
        config = get_config();
        fee_map = variant_to_map<name, asset>(config["fees"]);
        time_map = variant_to_map<name, uint32_t>(config["times"]);

        validate_map(fee_map, name("archival"), new_archival_fee);
        validate_map(time_map, name("minballength"), new_min_bal_length);

    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( transfer_handling, trail_tester ) try {
        string version = "v2.0.0-RC1";
        set_config(version, true);
        //trail balance is zero for testa, because a "skip" memo is provided
        const asset initial_balance = base_tester::get_currency_balance(token_name, tlos_sym, testa);
        const asset transfer_amount = asset::from_string("1.0000 TLOS");

        base_tester::transfer(testa, trail_name, "1.0000 TLOS", "skip", token_name);
        
        //check trail balance is 0 for testa
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, testa), asset::from_string("0.0000 TLOS"));

        //check token balance of testa is initial - 1
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, testa), initial_balance - transfer_amount);

        //check token balance of trail is 1
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, trail_name), transfer_amount);

        auto config = get_config();
        BOOST_REQUIRE_EQUAL(config["total_deposits"].as<asset>(), asset::from_string("0.0000 TLOS"));

        //trail balance for testa a should be 1, because "skip" was not supplied
        base_tester::transfer(testa, trail_name, "1.0000 TLOS", "literally anything else", token_name);

        //check trail balance is 1 for testa
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, testa), transfer_amount);

        //check token balance of testa is initial - 2
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, testa), initial_balance - (transfer_amount + transfer_amount));

        //check token balance of trail is 2
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, trail_name), transfer_amount + transfer_amount);

        config = get_config();
        BOOST_REQUIRE_EQUAL(config["total_deposits"].as<asset>(), asset::from_string("1.0000 TLOS"));

        //should fail because total_transferable < quantity in catch_transfer
        BOOST_REQUIRE_EXCEPTION(base_tester::transfer(trail_name, testa, "2.0000 TLOS", "transfer amount too large", token_name),
            eosio_assert_message_exception, eosio_assert_message_is( "trailservice lacks the liquid TLOS to make this transfer" ) 
        );

        //should succeed because it is less than transferable amount
        base_tester::transfer(trail_name, testb, "1.0000 TLOS", "normal transfer", token_name);

        //should fail because total_transferable < quantity in catch_transfer
        BOOST_REQUIRE_EXCEPTION(base_tester::transfer(trail_name, testa, "1.0000 TLOS", "transfer amount too large", token_name),
            eosio_assert_message_exception, eosio_assert_message_is( "trailservice lacks the liquid TLOS to make this transfer" ) 
        );
        
        //testa should still able able to withdraw its deposit
        auto trace = withdraw(testa, asset::from_string("1.0000 TLOS"));

        //NOTE: use for checking broadcast action
        BOOST_REQUIRE(std::find_if(trace->action_traces.begin(), trace->action_traces.end(), [](action_trace trace) {
            return trace.act.name == name("transfer") && trace.act.account == name("eosio.token");
        }) != trace->action_traces.end());

    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( treasury_basics, trail_tester ) try {
        asset max_supply = asset::from_string("1000000 FART");
        
        //fails because setconfig must be called
        BOOST_REQUIRE_EXCEPTION(new_treasury(testa, max_supply, name("public")),
            eosio_assert_message_exception, eosio_assert_message_is( "trailservice::setconfig must be called before treasuries can be emplaced" ) 
        );

        //set config 
        set_config("v2.0.0-RC1", true);

        //fails because there isn't a local balance for the fee
        BOOST_REQUIRE_EXCEPTION(new_treasury(testa, max_supply, name("public")),
            eosio_assert_message_exception, eosio_assert_message_is( "TLOS balance not found" ) 
        );
        produce_blocks();

        base_tester::transfer(testa, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(testb, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(testc, trail_name, "3000.0000 TLOS", "", token_name);
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, testa), asset::from_string("3000.0000 TLOS"));
        new_treasury(testa, max_supply, name("public"));
        
        //validate treasury structure
        fc::variant treasury = get_treasury(max_supply.get_symbol()).as<mvo>();

        map<name, bool> settings = variant_to_map<name, bool>(treasury["settings"]);

        validate_map(settings, name("burnable"), false);
        validate_map(settings, name("maxmutable"), false);
        validate_map(settings, name("reclaimable"), false);
        validate_map(settings, name("stakeable"), false);
        validate_map(settings, name("transferable"), false);
        validate_map(settings, name("unstakeable"), false);

        BOOST_REQUIRE_EQUAL(treasury["max_supply"].as<asset>(), max_supply);
        BOOST_REQUIRE_EQUAL(treasury["supply"].as<asset>(), asset::from_string("0 FART"));
        BOOST_REQUIRE_EQUAL(treasury["access"].as<name>(), name("public"));
        BOOST_REQUIRE_EQUAL(treasury["manager"].as<name>(), testa);
        BOOST_REQUIRE_EQUAL(treasury["title"], "");
        BOOST_REQUIRE_EQUAL(treasury["description"], "");
        BOOST_REQUIRE_EQUAL(treasury["icon"], "");
        BOOST_REQUIRE_EQUAL(treasury["voters"], uint32_t(0));
        BOOST_REQUIRE_EQUAL(treasury["delegates"], uint32_t(0));
        BOOST_REQUIRE_EQUAL(treasury["committees"], uint32_t(0));
        BOOST_REQUIRE_EQUAL(treasury["open_ballots"], uint32_t(0));
        BOOST_REQUIRE_EQUAL(treasury["locked"], false);
        BOOST_REQUIRE_EQUAL(treasury["unlock_acct"].as<name>(), testa);
        BOOST_REQUIRE_EQUAL(treasury["unlock_auth"].as<name>(), name("active"));

        //flip all settings and validate
        toggle(testa, max_supply.get_symbol(), name("burnable"));
        toggle(testa, max_supply.get_symbol(), name("maxmutable"));
        toggle(testa, max_supply.get_symbol(), name("reclaimable"));
        toggle(testa, max_supply.get_symbol(), name("stakeable"));
        toggle(testa, max_supply.get_symbol(), name("transferable"));
        toggle(testa, max_supply.get_symbol(), name("unstakeable"));

        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();

        settings = variant_to_map<name, bool>(treasury["settings"]);

        validate_map(settings, name("burnable"), true);
        validate_map(settings, name("maxmutable"), true);
        validate_map(settings, name("reclaimable"), true);
        validate_map(settings, name("stakeable"), true);
        validate_map(settings, name("transferable"), true);
        validate_map(settings, name("unstakeable"), true);

        //regvoter and validate
        reg_voter(testb, max_supply.get_symbol(), {});

        //mint tokens
        mint(testa, testb, max_supply - asset::from_string("1 FART"), "init amount");

        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();

        BOOST_REQUIRE_EQUAL(treasury["supply"].as<asset>(), max_supply - asset::from_string("1 FART"));

        validate_voter(testb, max_supply.get_symbol(), mvo()
            ("liquid", max_supply - asset::from_string("1 FART"))
            ("staked", "0 FART")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 FART")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );

        BOOST_REQUIRE_EXCEPTION(mint(testa, testb, asset::from_string("2 FART"), "an amount to go over max supply"),
            eosio_assert_message_exception, eosio_assert_message_is( "minting would breach max supply" ) 
        );

        BOOST_REQUIRE_EXCEPTION(transfer(testb, testc, asset::from_string("1000 FART"), "should fail, testc isn't registered"), 
            eosio_assert_message_exception, eosio_assert_message_is( "add_liquid: voter not found" ) 
        );

        produce_blocks();

        reg_voter(testc, max_supply.get_symbol(), {});

        validate_voter(testc, max_supply.get_symbol(), mvo()
            ("liquid", asset::from_string("0 FART"))
            ("staked", "0 FART")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 FART")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );

        //transfer to testc should succeed now that the account is registered
        transfer(testb, testc, asset::from_string("1000 FART"), "");

        validate_voter(testc, max_supply.get_symbol(), mvo()
            ("liquid", asset::from_string("1000 FART"))
            ("staked", "0 FART")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 FART")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );
        
        //attempt to burn tokens the manage has not yet reclaimed, should fail because manager doesn't have a voter emplacement
        BOOST_REQUIRE_EXCEPTION(burn(testa, asset::from_string("100 FART"), "should fail"), 
            eosio_assert_message_exception, eosio_assert_message_is( "manager voter not found" ) 
        );

        //register testa to its own treasury
        auto trace = reg_voter(testa, max_supply.get_symbol(), {});

        validate_action_payer(trace, trail_name, name("regvoter"), testa);

        validate_voter(testa, max_supply.get_symbol(), mvo()
            ("liquid", asset::from_string("0 FART"))
            ("staked", "0 FART")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 FART")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );

        produce_blocks();

        //attempt to burn again, should fail because manager doesn't possess the tokens
        BOOST_REQUIRE_EXCEPTION(burn(testa, asset::from_string("100 FART"), "should fail"), 
            eosio_assert_message_exception, eosio_assert_message_is( "burning would overdraw balance" ) 
        );

        //reclaim tokens from testb
        reclaim(testa, testb, asset::from_string("100 FART"), "because you made me angry");
        validate_voter(testa, max_supply.get_symbol(), mvo()
            ("liquid", asset::from_string("100 FART"))
            ("staked", "0 FART")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 FART")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );

        BOOST_REQUIRE_EXCEPTION(burn(testa, asset::from_string("101 FART"), "should fail"), 
            eosio_assert_message_exception, eosio_assert_message_is( "burning would overdraw balance" ) 
        );

        burn(testa, asset::from_string("100 FART"), "should pass");
        validate_voter(testa, max_supply.get_symbol(), mvo()
            ("liquid", asset::from_string("0 FART"))
            ("staked", "0 FART")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 FART")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );

        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();

        BOOST_REQUIRE_EQUAL(treasury["supply"].as<asset>(), max_supply - asset::from_string("1 FART") - asset::from_string("100 FART"));
        asset new_max_supply = max_supply - asset::from_string("100 FART");
        mutate_max(testa, new_max_supply, "");

        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();
        BOOST_REQUIRE_EQUAL(treasury["max_supply"].as<asset>(), new_max_supply);

        // change unlocker
        set_unlocker(testa, max_supply.get_symbol(), testb, name("fart"));
        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();

        BOOST_REQUIRE_EQUAL(treasury["unlock_acct"].as<name>(), testb);
        BOOST_REQUIRE_EQUAL(treasury["unlock_auth"].as<name>(), name("fart"));

        set_unlocker(testa, max_supply.get_symbol(), testa, name("active"));

        // lock
        lock(testa, max_supply.get_symbol());
        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();

        BOOST_REQUIRE_EQUAL(treasury["locked"].as<bool>(), true);

        // unlock
        unlock(testa, max_supply.get_symbol());
        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();

        BOOST_REQUIRE_EQUAL(treasury["locked"].as<bool>(), false);

        //create a new treasury, register a voter with referrer, referrer should be manager, and should pay for ram
        asset test_asset = asset::from_string("1000.000 TST");

        new_treasury(testa, test_asset, name("private"));

        //shouldn't be able to register voter for this treasury with a referrer who isn't the manager
        BOOST_REQUIRE_EXCEPTION(reg_voter(testb, test_asset.get_symbol(), testc), 
            eosio_assert_message_exception, eosio_assert_message_is( "referrer must be treasury manager" ) 
        );

        trace = reg_voter(testb, test_asset.get_symbol(), testa);

        validate_action_payer(trace, trail_name, name("regvoter"), testa);

        //attempt to create a new treasury with membership access, should fail
        test_asset = asset::from_string("1000.00 TT");

        BOOST_REQUIRE_EXCEPTION(new_treasury(testb, test_asset, name("membership")), 
            eosio_assert_message_exception, eosio_assert_message_is( "membership access feature under development" ) 
        );

        //create treasury with invite access, validate payer
        test_asset = asset::from_string("1000.0 T");

        new_treasury(testc, test_asset, name("invite"));

        trace = reg_voter(testa, test_asset.get_symbol(), testc);

        validate_action_payer(trace, trail_name, name("regvoter"), testc);

    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( ballot_basics, trail_tester ) try {
        //setup treasury
        set_config("v2.0.0-RC1", true);
        asset max_supply = asset::from_string("1000000.00 GOO");
        symbol treasury_symbol = max_supply.get_symbol();
        name ballot_name = name("ballot1");
        name valid_category = name("proposal");
        name invalid_category = name("invalid");

        name valid_voting_method = name("1acct1vote");
        name invalid_voting_method = name("invalidvote");

        name manager = name("manager");
        name voter1 = testa, voter2 = testb, voter3 = testc;

        create_account_with_resources(manager, eosio_name, asset::from_string("400.0000 TLOS"), false);
        base_tester::transfer(eosio_name, manager, "10000.0000 TLOS", "initial funds", token_name);

        base_tester::transfer(voter1, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter2, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter3, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(manager, trail_name, "5000.0000 TLOS", "", token_name);
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, voter1), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, voter2), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, voter3), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, manager), asset::from_string("5000.0000 TLOS"));
        
        new_treasury(manager, max_supply, name("public"));

        reg_voter(voter1, treasury_symbol, {});
        reg_voter(voter2, treasury_symbol, {});
        reg_voter(voter3, treasury_symbol, {});

        mint(manager, voter1, asset::from_string("1000.00 GOO"), "init amount");
        mint(manager, voter2, asset::from_string("1000.00 GOO"), "init amount");
        mint(manager, voter3, asset::from_string("1000.00 GOO"), "init amount");

        BOOST_REQUIRE_EXCEPTION(new_ballot(ballot_name, invalid_category, voter1, treasury_symbol, valid_voting_method, {}), 
            eosio_assert_message_exception, eosio_assert_message_is( "invalid category" ) 
        );

        BOOST_REQUIRE_EXCEPTION(new_ballot(ballot_name, valid_category, voter1, treasury_symbol, invalid_voting_method, {}), 
            eosio_assert_message_exception, eosio_assert_message_is( "invalid voting method" ) 
        );

        new_ballot(ballot_name, valid_category, voter1, treasury_symbol, valid_voting_method, { name("jonanyname")});

        //validate newly created ballot
        fc::variant ballot_info = get_ballot(ballot_name);
        BOOST_REQUIRE_EQUAL(ballot_info["ballot_name"].as<name>(), ballot_name);
        BOOST_REQUIRE_EQUAL(ballot_info["category"].as<name>(), valid_category);
        BOOST_REQUIRE_EQUAL(ballot_info["publisher"].as<name>(), voter1);
        BOOST_REQUIRE_EQUAL(ballot_info["status"].as<name>(), name("setup"));
        BOOST_REQUIRE_EQUAL(ballot_info["title"], "");
        BOOST_REQUIRE_EQUAL(ballot_info["description"], "");
        BOOST_REQUIRE_EQUAL(ballot_info["content"], "");
        BOOST_REQUIRE_EQUAL(ballot_info["treasury_symbol"].as<symbol>(), treasury_symbol);
        BOOST_REQUIRE_EQUAL(ballot_info["voting_method"].as<name>(), valid_voting_method);
        BOOST_REQUIRE_EQUAL(ballot_info["min_options"].as<uint32_t>(), uint32_t(1));
        BOOST_REQUIRE_EQUAL(ballot_info["min_options"].as<uint32_t>(), uint32_t(1));
        BOOST_REQUIRE_EQUAL(ballot_info["total_voters"].as<uint32_t>(), uint32_t(0));
        BOOST_REQUIRE_EQUAL(ballot_info["total_delegates"].as<uint32_t>(), uint32_t(0));
        BOOST_REQUIRE_EQUAL(ballot_info["cleaned_count"].as<uint32_t>(), uint32_t(0));
        BOOST_REQUIRE_EQUAL(ballot_info["begin_time"], "1970-01-01T00:00:00");
        BOOST_REQUIRE_EQUAL(ballot_info["end_time"], "1970-01-01T00:00:00");
        
        map<name, asset> options_map = variant_to_map<name, asset>(ballot_info["options"]);
        map<name, bool> settings_map = variant_to_map<name, bool>(ballot_info["settings"]);

        validate_map(options_map, name("jonanyname"), asset::from_string("0.00 GOO"));

        validate_map(settings_map, name("lightballot"), false);
        validate_map(settings_map, name("revotable"), true);
        validate_map(settings_map, name("votestake"), true);
        validate_map(settings_map, name("writein"), false);

        //edit ballot details and verify
        string title = "The Title of My Ballot";
        string description = "to determine the future of this country";
        string content = "the content";
        edit_details(voter1, ballot_name, title, description, content);
        ballot_info = get_ballot(ballot_name);
        BOOST_REQUIRE_EQUAL(ballot_info["title"], title);
        BOOST_REQUIRE_EQUAL(ballot_info["description"], description);
        BOOST_REQUIRE_EQUAL(ballot_info["content"], content);

        //edit min max
        
        BOOST_REQUIRE_EXCEPTION(edit_min_max(voter1, ballot_name, 0, 0), 
            eosio_assert_message_exception, eosio_assert_message_is( "min and max options must be greater than zero" ) 
        );

        BOOST_REQUIRE_EXCEPTION(edit_min_max(voter1, ballot_name, 2, 1), 
            eosio_assert_message_exception, eosio_assert_message_is( "max must be greater than or equal to min" ) 
        );

        BOOST_REQUIRE_EXCEPTION(edit_min_max(voter1, ballot_name, 1, 2), 
            eosio_assert_message_exception, eosio_assert_message_is( "max options cannot be greater than number of options" ) 
        );
        
        produce_blocks();

        //cant change max until another option is added
        name new_option_name = name("everyman");
        add_option(voter1, ballot_name, new_option_name);

        ballot_info = get_ballot(ballot_name);
        options_map = variant_to_map<name, asset>(ballot_info["options"]);

        validate_map(options_map, new_option_name, asset::from_string("0.00 GOO"));

        produce_blocks();

        BOOST_REQUIRE_EXCEPTION(add_option(voter1, ballot_name, new_option_name), 
            eosio_assert_message_exception, eosio_assert_message_is( "option is already in ballot" ) 
        );

        edit_min_max(voter1, ballot_name, 2, 2);

        ballot_info = get_ballot(ballot_name);

        BOOST_REQUIRE_EQUAL(ballot_info["min_options"].as<uint8_t>(), uint8_t(2));
        BOOST_REQUIRE_EQUAL(ballot_info["max_options"].as<uint8_t>(), uint8_t(2));
        
        //remove option
        BOOST_REQUIRE_EXCEPTION(rmv_option(voter1, ballot_name, name("unknown")), 
            eosio_assert_message_exception, eosio_assert_message_is( "option not found" ) 
        );

        rmv_option(voter1, ballot_name, new_option_name);
        ballot_info = get_ballot(ballot_name);
        options_map = variant_to_map<name, asset>(ballot_info["options"]);
        BOOST_REQUIRE_EQUAL(options_map.count(new_option_name), 0);

        //TODO: validate if options changed, check with craig

        //togglebal settings
        BOOST_REQUIRE_EXCEPTION(toggle_bal(voter1, ballot_name, name("lightballo")), 
            eosio_assert_message_exception, eosio_assert_message_is( "setting not found" ) 
        );

        toggle_bal(voter1, ballot_name, name("lightballot"));

        settings_map = variant_to_map<name, bool>(get_ballot(ballot_name)["settings"]);

        validate_map(settings_map, name("lightballot"), true);

        produce_blocks();

        toggle_bal(voter1, ballot_name, name("lightballot"));

        settings_map = variant_to_map<name, bool>(get_ballot(ballot_name)["settings"]);

        validate_map(settings_map, name("lightballot"), false);

        //attempt to cancel before opening
        BOOST_REQUIRE_EXCEPTION(cancel_ballot(voter1, ballot_name, "because nevermind"), 
            eosio_assert_message_exception, eosio_assert_message_is( "ballot must be in voting mode to cancel" ) 
        );

        //TODO: attempt to delete ballot

        //open for voting
        BOOST_REQUIRE_EXCEPTION(open_voting(voter1, ballot_name, get_current_time_point_sec()), 
            eosio_assert_message_exception, eosio_assert_message_is( "ballot must have at least 2 options" ) 
        );

        add_option(voter1, ballot_name, new_option_name);

        BOOST_REQUIRE_EXCEPTION(open_voting(voter1, ballot_name, get_current_time_point_sec()), 
            eosio_assert_message_exception, eosio_assert_message_is( "end time must be in the future" ) 
        );

        produce_blocks();

        BOOST_REQUIRE_EXCEPTION(open_voting(voter1, ballot_name, get_current_time_point_sec() + uint32_t(10)), 
            eosio_assert_message_exception, eosio_assert_message_is( "ballot must be open for minimum ballot length" ) 
        );
        
        fc::variant config = get_config();
        uint32_t min_bal_length = variant_to_map<name, uint32_t>(config["times"])[name("minballength")];

        BOOST_REQUIRE_EXCEPTION(open_voting(voter1, ballot_name, get_current_time_point_sec() + min_bal_length - 1), 
            eosio_assert_message_exception, eosio_assert_message_is( "ballot must be open for minimum ballot length" ) 
        );

        produce_blocks();

        open_voting(voter1, ballot_name, get_current_time_point_sec() + min_bal_length + 10);

        ballot_info = get_ballot(ballot_name);
        BOOST_REQUIRE_EQUAL(ballot_info["status"].as<name>(), name("voting"));

        // NOTE: these validation fail intermittently. There seems to be a difference between on chain time functions and these functions
        // BOOST_REQUIRE_EQUAL(ballot_info["begin_time"], (time_point_sec(get_current_time_point())).to_iso_string());
        // BOOST_REQUIRE_EQUAL(ballot_info["end_time"], (time_point_sec(get_current_time_point()) + min_bal_length).to_iso_string());


        //then cancel
        fc::variant treasury_info = get_treasury(treasury_symbol);
        BOOST_REQUIRE_EQUAL(treasury_info["open_ballots"].as<uint32_t>(), uint32_t(1));

        cancel_ballot(voter1, ballot_name, "because nevermind");
        ballot_info = get_ballot(ballot_name);
        BOOST_REQUIRE_EQUAL(ballot_info["status"].as<name>(), name("cancelled"));

        treasury_info = get_treasury(treasury_symbol);
        BOOST_REQUIRE_EQUAL(treasury_info["open_ballots"].as<uint32_t>(), uint32_t(0));
        
        //then delete
        BOOST_REQUIRE_EXCEPTION(delete_ballot(voter1, ballot_name), 
            eosio_assert_message_exception, eosio_assert_message_is( "cannot delete until 5 days past ballot's end time" ) 
        );

        produce_blocks();
        produce_block(fc::days(10));
        produce_blocks();

        delete_ballot(voter1, ballot_name);
        ballot_info = get_ballot(ballot_name);
        BOOST_REQUIRE(ballot_info.is_null());

    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( ballot_setup_restrictions, trail_tester ) try {
        set_config("v2.0.0-RC1", true);
        asset max_supply = asset::from_string("1000000.00 GOO");
        symbol treasury_symbol = max_supply.get_symbol();
        name ballot_name = name("ballot1");
        name category = name("proposal");
        name voting_method = name("1acct1vote");

        name manager = name("manager");
        name voter1 = testa, voter2 = testb, voter3 = testc;

        create_account_with_resources(manager, eosio_name, asset::from_string("400.0000 TLOS"), false);
        base_tester::transfer(eosio_name, manager, "10000.0000 TLOS", "initial funds", token_name);

        base_tester::transfer(voter1, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter2, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter3, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(manager, trail_name, "5000.0000 TLOS", "", token_name);
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, voter1), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, voter2), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, voter3), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, manager), asset::from_string("5000.0000 TLOS"));
        
        new_treasury(manager, max_supply, name("public"));

        reg_voter(voter1, treasury_symbol, {});
        reg_voter(voter2, treasury_symbol, {});
        reg_voter(voter3, treasury_symbol, {});

        mint(manager, voter1, asset::from_string("1000.00 GOO"), "init amount");
        mint(manager, voter2, asset::from_string("1000.00 GOO"), "init amount");
        mint(manager, voter3, asset::from_string("1000.00 GOO"), "init amount");

        new_ballot(ballot_name, category, voter1, treasury_symbol, voting_method, { name("option1"), name("option2") });

        fc::variant config = get_config();
        uint32_t min_bal_length = variant_to_map<name, uint32_t>(config["times"])[name("minballength")];

        open_voting(voter1, ballot_name, get_current_time_point_sec() + min_bal_length + 10);
        
        BOOST_REQUIRE_EXCEPTION(edit_details(voter1, ballot_name, "new title", "new description", "new content"), 
            eosio_assert_message_exception, eosio_assert_message_is( "ballot must be in setup mode to edit details" ) 
        );

        BOOST_REQUIRE_EXCEPTION(toggle_bal(voter1, ballot_name, name("lightballot")), 
            eosio_assert_message_exception, eosio_assert_message_is( "ballot must be in setup mode to toggle settings" ) 
        );

        BOOST_REQUIRE_EXCEPTION(edit_min_max(voter1, ballot_name, 2, 2), 
            eosio_assert_message_exception, eosio_assert_message_is( "ballot must be in setup mode to edit max options" ) 
        );

        BOOST_REQUIRE_EXCEPTION(rmv_option(voter1, ballot_name, name("options2")), 
            eosio_assert_message_exception, eosio_assert_message_is( "ballot must be in setup mode to remove options" ) 
        );

        BOOST_REQUIRE_EXCEPTION(add_option(voter1, ballot_name, name("options3")), 
            eosio_assert_message_exception, eosio_assert_message_is( "ballot must be in setup mode to add options" ) 
        );
    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( committee_basics, trail_tester ) try {
        set_config("v2.0.0-RC1", true);
        asset max_supply = asset::from_string("1000000.00 GOO");
        symbol treasury_symbol = max_supply.get_symbol();
        name ballot_name = name("ballot1");
        name category = name("proposal");
        name one_account_one_vote = name("1acct1vote");
        name one_token_n_vote = name("1tokennvote");
        name one_token_one_vote = name("1token1vote");
        name one_token_square_one_vote = name("1tsquare1v");
        name quadratic = name("quadratic");

        name manager = name("manager");
        name voter1 = testa, voter2 = testb, voter3 = testc;

        create_account_with_resources(manager, eosio_name, asset::from_string("400.0000 TLOS"), false);
        base_tester::transfer(eosio_name, manager, "10000.0000 TLOS", "initial funds", token_name);

        base_tester::transfer(voter1, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter2, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter3, trail_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(manager, trail_name, "5000.0000 TLOS", "", token_name);
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, voter1), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, voter2), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, voter3), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(trail_name, tlos_sym, manager), asset::from_string("5000.0000 TLOS"));
        
        new_treasury(manager, max_supply, name("public"));

        reg_voter(voter1, treasury_symbol, {});
        reg_voter(voter2, treasury_symbol, {});
        reg_voter(voter3, treasury_symbol, {});

        mint(manager, voter1, asset::from_string("1000.00 GOO"), "init amount");
        mint(manager, voter2, asset::from_string("1000.00 GOO"), "init amount");
        mint(manager, voter3, asset::from_string("1000.00 GOO"), "init amount");

        new_ballot(one_account_one_vote, category, voter1, treasury_symbol, one_account_one_vote, { name("option1"), name("option2") });
        new_ballot(one_token_n_vote, category, voter1, treasury_symbol, one_token_n_vote, { name("option1"), name("option2") });
        new_ballot(one_token_one_vote, category, voter1, treasury_symbol, one_token_one_vote, { name("option1"), name("option2") });
        new_ballot(one_token_square_one_vote, category, voter1, treasury_symbol, one_token_square_one_vote, { name("option1"), name("option2") });
        new_ballot(quadratic, category, voter1, treasury_symbol, quadratic, { name("option1"), name("option2") });
        
        
    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( worker_basics, trail_tester ) try {
        // test committee actions
    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( payroll_basics, trail_tester ) try {
        // test payroll actions
    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( voting_methods, trail_tester ) try {
        //vote on a ballot with each voting_method: maybe another test
    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( full_flow, trail_tester ) try {
        //run through a common use case for trail

        //postresults

        //broadcast
    } FC_LOG_AND_RETHROW()
    
BOOST_AUTO_TEST_SUITE_END()