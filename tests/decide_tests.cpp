#include <boost/test/unit_test.hpp>
#include <eosio/testing/tester.hpp>
#include <fc/variant_object.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <iostream>
#include <boost/container/map.hpp>
#include <map>

#include <Runtime/Runtime.h>
#include <iomanip>

#include "decide_tester.hpp"

using namespace eosio;
using namespace eosio::testing;
using namespace eosio::chain;
using namespace fc;
using namespace std;
using namespace decidetesting::testing;
using mvo = fc::mutable_variant_object;

BOOST_AUTO_TEST_SUITE(decide_tests)

    BOOST_FIXTURE_TEST_CASE( config_tests, decide_tester ) try {

        //======================== check config state from setup ========================

        //initialize
        string app_name = "Telos Decide";
        string app_version = "v2.0.0";

        //get table data
        fc::variant config = get_config();
        map<name, asset> fee_map = variant_to_map<name, asset>(config["fees"]);
        map<name, uint32_t> time_map = variant_to_map<name, uint32_t>(config["times"]);

        //assert config values
        BOOST_REQUIRE_EQUAL(config["app_name"], app_name);
        BOOST_REQUIRE_EQUAL(config["app_version"], app_version);
        BOOST_REQUIRE_EQUAL(config["total_deposits"].as<asset>(), asset::from_string("0.0000 TLOS"));
        
        validate_map(fee_map, name("archival"), asset::from_string("1.0000 TLOS"));
        validate_map(fee_map, name("ballot"), asset::from_string("10.0000 TLOS"));
        validate_map(fee_map, name("committee"), asset::from_string("10.0000 TLOS"));
        validate_map(fee_map, name("treasury"), asset::from_string("500.0000 TLOS"));

        validate_map(time_map, name("minballength"), uint32_t(60));
        validate_map(time_map, name("balcooldown"), uint32_t(86400));
        validate_map(time_map, name("forfeittime"), uint32_t(864000));

        //======================== attempt invalid actions ========================
        
        //attempt fee change with improper symbol
        BOOST_REQUIRE_EXCEPTION(update_fee(name("archival"), asset::from_string("100.0000 TST")),
            eosio_assert_message_exception, eosio_assert_message_is( "fee symbol must be TLOS" ) 
        );

        //attempt time change with a zero time
        BOOST_REQUIRE_EXCEPTION(update_time(name("balcooldown"), uint32_t(0)),
            eosio_assert_message_exception, eosio_assert_message_is( "length must be a positive number" ) 
        );

        //======================== change fee amount ========================

        //change a fee and a time
        asset new_archival_fee = asset::from_string("100.0000 TLOS");

        //send updatefee action
        update_fee(name("archival"), new_archival_fee);
        produce_blocks();

        //get table data
        config = get_config();
        fee_map = variant_to_map<name, asset>(config["fees"]);

        //assert table values
        validate_map(fee_map, name("archival"), new_archival_fee);

        //======================== change time amount ========================

        //initialize
        uint32_t new_min_bal_length = uint32_t(1000);

        //send updatetime action
        update_time(name("minballength"), new_min_bal_length);
        produce_blocks();

        //get table data
        config = get_config();
        time_map = variant_to_map<name, uint32_t>(config["times"]);

        //asset table values
        validate_map(time_map, name("minballength"), new_min_bal_length);

        //======================== change app version ========================

        //initialize
        string new_app_version = "v2.0.1";

        //send setversion action
        decide_setversion(new_app_version);

        //get table data
        config = get_config();

        //assert table values
        BOOST_REQUIRE_EQUAL(config["app_version"], new_app_version);

    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( transfer_handling, decide_tester ) try {
        
        //trail balance is zero for testa, because a "skip" memo is provided
        const asset initial_balance = base_tester::get_currency_balance(token_name, tlos_sym, testa);
        const asset transfer_amount = asset::from_string("1.0000 TLOS");

        base_tester::transfer(testa, decide_name, "1.0000 TLOS", "skip", token_name);
        
        //check trail balance is 0 for testa
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, testa), asset::from_string("0.0000 TLOS"));

        //check token balance of testa is initial - 1
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, testa), initial_balance - transfer_amount);

        //check token balance of trail is 1
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, decide_name), transfer_amount);

        auto config = get_config();
        BOOST_REQUIRE_EQUAL(config["total_deposits"].as<asset>(), asset::from_string("0.0000 TLOS"));

        //trail balance for testa a should be 1, because "skip" was not supplied
        base_tester::transfer(testa, decide_name, "1.0000 TLOS", "literally anything else", token_name);

        //check trail balance is 1 for testa
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, testa), transfer_amount);

        //check token balance of testa is initial - 2
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, testa), initial_balance - (transfer_amount + transfer_amount));

        //check token balance of trail is 2
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, decide_name), transfer_amount + transfer_amount);

        config = get_config();
        BOOST_REQUIRE_EQUAL(config["total_deposits"].as<asset>(), asset::from_string("1.0000 TLOS"));

        //should fail because total_transferable < quantity in catch_transfer
        BOOST_REQUIRE_EXCEPTION(base_tester::transfer(decide_name, testa, "2.0000 TLOS", "transfer amount too large", token_name),
            eosio_assert_message_exception, eosio_assert_message_is( "Telos Decide lacks the liquid TLOS to make this transfer" ) 
        );

        //should succeed because it is less than transferable amount
        base_tester::transfer(decide_name, testb, "1.0000 TLOS", "normal transfer", token_name);

        //should fail because total_transferable < quantity in catch_transfer
        BOOST_REQUIRE_EXCEPTION(base_tester::transfer(decide_name, testa, "1.0000 TLOS", "transfer amount too large", token_name),
            eosio_assert_message_exception, eosio_assert_message_is( "Telos Decide lacks the liquid TLOS to make this transfer" ) 
        );
        
        //testa should still able able to withdraw its deposit
        auto trace = withdraw(testa, asset::from_string("1.0000 TLOS"));

        //NOTE: use for checking broadcast action
        BOOST_REQUIRE(std::find_if(trace->action_traces.begin(), trace->action_traces.end(), [](action_trace trace) {
            return trace.act.name == name("transfer") && trace.act.account == name("eosio.token");
        }) != trace->action_traces.end());

    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( treasury_basics, decide_tester ) try {
        
        //initialize
        asset max_supply = asset::from_string("1000000 DECIDE");

        //fails because there isn't a local balance for the fee
        BOOST_REQUIRE_EXCEPTION(new_treasury(testa, max_supply, name("public")),
            eosio_assert_message_exception, eosio_assert_message_is( "TLOS balance not found" ) 
        );
        produce_blocks();

        base_tester::transfer(testa, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(testb, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(testc, decide_name, "3000.0000 TLOS", "", token_name);
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, testa), asset::from_string("3000.0000 TLOS"));
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
        BOOST_REQUIRE_EQUAL(treasury["supply"].as<asset>(), asset::from_string("0 DECIDE"));
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
        mint(testa, testb, max_supply - asset::from_string("1 DECIDE"), "init amount");

        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();

        BOOST_REQUIRE_EQUAL(treasury["supply"].as<asset>(), max_supply - asset::from_string("1 DECIDE"));

        validate_voter(testb, max_supply.get_symbol(), mvo()
            ("liquid", max_supply - asset::from_string("1 DECIDE"))
            ("staked", "0 DECIDE")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 DECIDE")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );

        BOOST_REQUIRE_EXCEPTION(mint(testa, testb, asset::from_string("2 DECIDE"), "an amount to go over max supply"),
            eosio_assert_message_exception, eosio_assert_message_is( "minting would breach max supply" ) 
        );

        BOOST_REQUIRE_EXCEPTION(transfer(testb, testc, asset::from_string("1000 DECIDE"), "should fail, testc isn't registered"), 
            eosio_assert_message_exception, eosio_assert_message_is( "add_liquid: voter not found" ) 
        );

        produce_blocks();

        reg_voter(testc, max_supply.get_symbol(), {});

        validate_voter(testc, max_supply.get_symbol(), mvo()
            ("liquid", asset::from_string("0 DECIDE"))
            ("staked", "0 DECIDE")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 DECIDE")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );

        //transfer to testc should succeed now that the account is registered
        transfer(testb, testc, asset::from_string("1000 DECIDE"), "");

        validate_voter(testc, max_supply.get_symbol(), mvo()
            ("liquid", asset::from_string("1000 DECIDE"))
            ("staked", "0 DECIDE")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 DECIDE")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );
        
        //attempt to burn tokens the manage has not yet reclaimed, should fail because manager doesn't have a voter emplacement
        BOOST_REQUIRE_EXCEPTION(burn(testa, asset::from_string("100 DECIDE"), "should fail"), 
            eosio_assert_message_exception, eosio_assert_message_is( "manager voter not found" ) 
        );

        //register testa to its own treasury
        auto trace = reg_voter(testa, max_supply.get_symbol(), {});

        validate_action_payer(trace, decide_name, name("regvoter"), testa);

        validate_voter(testa, max_supply.get_symbol(), mvo()
            ("liquid", asset::from_string("0 DECIDE"))
            ("staked", "0 DECIDE")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 DECIDE")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );

        produce_blocks();

        //attempt to burn again, should fail because manager doesn't possess the tokens
        BOOST_REQUIRE_EXCEPTION(burn(testa, asset::from_string("100 DECIDE"), "should fail"), 
            eosio_assert_message_exception, eosio_assert_message_is( "burning would overdraw balance" ) 
        );

        //reclaim tokens from testb
        reclaim(testa, testb, asset::from_string("100 DECIDE"), "because you made me angry");
        validate_voter(testa, max_supply.get_symbol(), mvo()
            ("liquid", asset::from_string("100 DECIDE"))
            ("staked", "0 DECIDE")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 DECIDE")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );

        BOOST_REQUIRE_EXCEPTION(burn(testa, asset::from_string("101 DECIDE"), "should fail"), 
            eosio_assert_message_exception, eosio_assert_message_is( "burning would overdraw balance" ) 
        );

        burn(testa, asset::from_string("100 DECIDE"), "should pass");
        validate_voter(testa, max_supply.get_symbol(), mvo()
            ("liquid", asset::from_string("0 DECIDE"))
            ("staked", "0 DECIDE")
            ("staked_time", time_point_sec(get_current_time_point()))
            ("delegated", "0 DECIDE")
            ("delegated_to", "")
            ("delegation_time", time_point_sec(get_current_time_point()))
        );

        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();

        BOOST_REQUIRE_EQUAL(treasury["supply"].as<asset>(), max_supply - asset::from_string("1 DECIDE") - asset::from_string("100 DECIDE"));
        asset new_max_supply = max_supply - asset::from_string("100 DECIDE");
        mutate_max(testa, new_max_supply, "");

        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();
        BOOST_REQUIRE_EQUAL(treasury["max_supply"].as<asset>(), new_max_supply);

        // change unlocker
        set_unlocker(testa, max_supply.get_symbol(), testb, name("decide"));
        treasury = get_treasury(max_supply.get_symbol()).as<mvo>();

        BOOST_REQUIRE_EQUAL(treasury["unlock_acct"].as<name>(), testb);
        BOOST_REQUIRE_EQUAL(treasury["unlock_auth"].as<name>(), name("decide"));

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

        validate_action_payer(trace, decide_name, name("regvoter"), testa);

        //attempt to create a new treasury with membership access, should fail
        test_asset = asset::from_string("1000.00 TT");

        BOOST_REQUIRE_EXCEPTION(new_treasury(testb, test_asset, name("membership")), 
            eosio_assert_message_exception, eosio_assert_message_is( "membership access feature under development" ) 
        );

        //create treasury with invite access, validate payer
        test_asset = asset::from_string("1000.0 T");

        new_treasury(testc, test_asset, name("invite"));

        //register referrer
        reg_voter(testc, test_asset.get_symbol(), {});

        //invite voter
        trace = reg_voter(testa, test_asset.get_symbol(), testc);

        validate_action_payer(trace, decide_name, name("regvoter"), testc);

        edit_trs_info(testa, max_supply.get_symbol(), "new title", "new description", "new icon");

        treasury = get_treasury(max_supply.get_symbol());

        BOOST_REQUIRE_EQUAL(treasury["title"], "new title");
        BOOST_REQUIRE_EQUAL(treasury["description"], "new description");
        BOOST_REQUIRE_EQUAL(treasury["icon"], "new icon");

        fc::variant payroll = get_payroll(max_supply.get_symbol(), name("workers"));
        fc::variant labor_bucket_info = get_labor_bucket(max_supply.get_symbol(), name("workers"));

        BOOST_REQUIRE(!payroll.is_null());
        BOOST_REQUIRE_EQUAL(payroll["payroll_funds"].as<asset>(), asset(0, tlos_sym));
        BOOST_REQUIRE_EQUAL(payroll["payroll_name"].as<name>(), name("workers"));
        BOOST_REQUIRE_EQUAL(payroll["period_length"].as<uint32_t>(), 604800);
        BOOST_REQUIRE_EQUAL(payroll["per_period"].as<asset>(), asset(10, tlos_sym));
        BOOST_REQUIRE_EQUAL(payroll["claimable_pay"].as<asset>(), asset(0, tlos_sym));
        BOOST_REQUIRE_EQUAL(payroll["payee"].as<name>(), name("workers"));


        BOOST_REQUIRE(!labor_bucket_info.is_null());
        BOOST_REQUIRE_EQUAL(labor_bucket_info["payroll_name"].as<name>(), name("workers"));
        map<name, asset> claimable_volume = variant_to_map<name, asset>(labor_bucket_info["claimable_volume"]);
        BOOST_REQUIRE_EQUAL(claimable_volume[name("rebalvolume")], asset(0, max_supply.get_symbol()));

        map<name, uint32_t> claimable_events = variant_to_map<name, uint32_t>(labor_bucket_info["claimable_events"]);
        BOOST_REQUIRE_EQUAL(claimable_events[name("rebalcount")], 0);
        BOOST_REQUIRE_EQUAL(claimable_events[name("cleancount")], 0);

        BOOST_REQUIRE_EXCEPTION(add_funds(eosio_name, eosio_name, max_supply.get_symbol(), name("workers"), asset::from_string("200.0000 GOO")), 
            eosio_assert_message_exception, eosio_assert_message_is( "only TLOS allowed in payrolls" ) 
        );

        BOOST_REQUIRE_EXCEPTION(edit_pay_rate(testa, name("workers"), max_supply.get_symbol(), 86400, asset::from_string("400.0000 GOO")),
            eosio_assert_message_exception, eosio_assert_message_is( "only TLOS allowed in payrolls" ) 
        );

    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( ballot_basics, decide_tester ) try {
        
        //initialize
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

        base_tester::transfer(voter1, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter2, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter3, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(manager, decide_name, "5000.0000 TLOS", "", token_name);
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter1), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter2), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter3), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, manager), asset::from_string("5000.0000 TLOS"));
        
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
        validate_map(settings_map, name("votestake"), false);

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

        //validate that min and max options were lowered the number options was less than their previous
        BOOST_REQUIRE_EQUAL(ballot_info["min_options"].as<uint8_t>(), uint8_t(1));
        BOOST_REQUIRE_EQUAL(ballot_info["max_options"].as<uint8_t>(), uint8_t(1));

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

    BOOST_FIXTURE_TEST_CASE( ballot_setup_restrictions, decide_tester ) try {
        

        asset max_supply = asset::from_string("1000000.00 GOO");
        symbol treasury_symbol = max_supply.get_symbol();
        name ballot_name = name("ballot1");
        name category = name("proposal");
        name voting_method = name("1acct1vote");

        name manager = name("manager");
        name voter1 = testa, voter2 = testb, voter3 = testc;

        create_account_with_resources(manager, eosio_name, asset::from_string("400.0000 TLOS"), false);
        base_tester::transfer(eosio_name, manager, "10000.0000 TLOS", "initial funds", token_name);

        base_tester::transfer(voter1, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter2, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter3, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(manager, decide_name, "5000.0000 TLOS", "", token_name);
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter1), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter2), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter3), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, manager), asset::from_string("5000.0000 TLOS"));
        
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

    BOOST_FIXTURE_TEST_CASE( committee_basics, decide_tester ) try {
        
        
        asset max_supply = asset::from_string("1000000.00 GOO");
        symbol treasury_symbol = max_supply.get_symbol();
        name ballot_name = name("ballot1");
        name category = name("proposal");

        name manager = name("manager");
        name voter1 = testa, voter2 = testb, voter3 = testc;

        create_account_with_resources(manager, eosio_name, asset::from_string("400.0000 TLOS"), false);
        base_tester::transfer(eosio_name, manager, "10000.0000 TLOS", "initial funds", token_name);

        base_tester::transfer(voter1, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter2, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter3, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(manager, decide_name, "5000.0000 TLOS", "", token_name);
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter1), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter2), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter3), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, manager), asset::from_string("5000.0000 TLOS"));
        
        new_treasury(manager, max_supply, name("public"));
        toggle(manager, treasury_symbol, name("stakeable"));

        reg_voter(voter1, treasury_symbol, {});
        reg_voter(voter2, treasury_symbol, {});
        reg_voter(voter3, treasury_symbol, {});

        name committee_name = name("tf");

        name seat1 = name("seat1"), seat2 = name("seat2"), seat3 = name("seat3"), seat4 = name("seat4");

        reg_committee(committee_name, "Telos Foundation", treasury_symbol, { 
            seat1,
            seat2
        }, voter1);

        fc::variant committee_info = get_committee(treasury_symbol, committee_name);
        map<name, name> seat_map = variant_to_map<name, name>(committee_info["seats"]);

        name updater = name("updater");

        create_account_with_resources(updater, eosio_name, asset::from_string("400.0000 TLOS"), false);

        BOOST_REQUIRE_EQUAL(seat_map.count(seat1), 1);
        BOOST_REQUIRE_EQUAL(seat_map[seat1], name(""));

        BOOST_REQUIRE_EQUAL(seat_map.count(seat2), 1);
        BOOST_REQUIRE_EQUAL(seat_map[seat2], name(""));

        set_updater({ { updater, name("active") }, { voter1, name("active") } }, committee_name, treasury_symbol, updater, name("active"));

        committee_info = get_committee(treasury_symbol, committee_name);

        BOOST_REQUIRE_EQUAL(committee_info["committee_title"], "Telos Foundation");
        BOOST_REQUIRE_EQUAL(committee_info["committee_name"].as<name>(), committee_name);

        BOOST_REQUIRE_EQUAL(committee_info["updater_acct"].as<name>(), updater);
        BOOST_REQUIRE_EQUAL(committee_info["updater_auth"].as<name>(), name("active"));

        add_seat(updater, committee_name, treasury_symbol, seat3);

        committee_info = get_committee(treasury_symbol, committee_name);
        seat_map = variant_to_map<name, name>(committee_info["seats"]);

        BOOST_REQUIRE_EQUAL(seat_map.count(seat3), 1);
        BOOST_REQUIRE_EQUAL(seat_map[seat3], name(""));

        add_seat(updater, committee_name, treasury_symbol, seat4);
        committee_info = get_committee(treasury_symbol, committee_name);
        seat_map = variant_to_map<name, name>(committee_info["seats"]);

        BOOST_REQUIRE_EQUAL(seat_map.count(seat4), 1);
        BOOST_REQUIRE_EQUAL(seat_map[seat4], name(""));

        remove_seat(updater, committee_name, treasury_symbol, seat4);

        committee_info = get_committee(treasury_symbol, committee_name);
        seat_map = variant_to_map<name, name>(committee_info["seats"]);

        BOOST_REQUIRE_EQUAL(seat_map.count(seat4), 0);

        assign_seat(updater, committee_name, treasury_symbol, seat1, voter1, "setting seat1");
        assign_seat(updater, committee_name, treasury_symbol, seat2, voter2, "setting seat2");
        assign_seat(updater, committee_name, treasury_symbol, seat3, voter3, "setting seat3");

        committee_info = get_committee(treasury_symbol, committee_name);
        seat_map = variant_to_map<name, name>(committee_info["seats"]);

        BOOST_REQUIRE_EQUAL(seat_map[seat1], voter1);
        BOOST_REQUIRE_EQUAL(seat_map[seat2], voter2);
        BOOST_REQUIRE_EQUAL(seat_map[seat3], voter3);
        
        del_committee(updater, committee_name, treasury_symbol, "damn the TF!");

        BOOST_REQUIRE(get_committee(treasury_symbol, committee_name).is_null());

    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( vote_symbol_staking, decide_tester ) try {
        
        
        asset max_supply = asset::from_string("1000000000.0000 VOTE");
        symbol treasury_symbol = max_supply.get_symbol();
        name ballot_name = name("ballot1");
        name category = name("proposal");

        name voting_method = name("1acct1vote");

        name voter1 = testa, voter2 = testb, voter3 = testc;

        base_tester::transfer(voter1, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter2, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter3, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(eosio_name, decide_name, "10000.0000 TLOS", "", token_name);
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter1), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter2), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter3), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, eosio_name), asset::from_string("10000.0000 TLOS"));
        
        new_treasury(eosio_name, max_supply, name("public"));

        delegate_bw(testa, testb, asset::from_string("1.0000 TLOS"), asset::from_string("1.0000 TLOS"), false);

        reg_voter(voter1, treasury_symbol, { });
        produce_blocks();
        //check that cpu + net quantity = total staked in trail
        fc::variant user_resource_info = get_user_res(testa);
        cout << user_resource_info << endl;

        asset current_stake = user_resource_info["net_weight"].as<asset>() + user_resource_info["cpu_weight"].as<asset>();
        int64_t staked_weight_amount = current_stake.get_amount();

        fc::variant voter_info = get_voter(voter1, treasury_symbol);

        BOOST_REQUIRE_EQUAL(voter_info["staked"].as<asset>().get_amount(), staked_weight_amount);
        
        //TODO: test with rex stake

        //delegate_bw validate old system stake increased and that staked in trail increases.
        asset stake_delta = asset::from_string("40.0000 TLOS");
        delegate_bw(voter1, voter1, stake_delta, stake_delta, false);
        user_resource_info = get_user_res(voter1);

        //current stake = initial + 2 * stake_delta
        BOOST_REQUIRE_EQUAL(staked_weight_amount + (stake_delta + stake_delta).get_amount(), 
            (user_resource_info["net_weight"].as<asset>() + user_resource_info["cpu_weight"].as<asset>()).get_amount());
        
        voter_info = get_voter(voter1, treasury_symbol);
        staked_weight_amount = (user_resource_info["net_weight"].as<asset>() + user_resource_info["cpu_weight"].as<asset>()).get_amount();
        BOOST_REQUIRE_EQUAL(voter_info["staked"].as<asset>().get_amount(), staked_weight_amount);

        undelegate_bw(voter1, voter1, stake_delta, stake_delta);
        user_resource_info = get_user_res(voter1);
        current_stake = user_resource_info["net_weight"].as<asset>() + user_resource_info["cpu_weight"].as<asset>();
        BOOST_REQUIRE_EQUAL(staked_weight_amount - (stake_delta + stake_delta).get_amount(), current_stake.get_amount());

        staked_weight_amount = staked_weight_amount - (stake_delta + stake_delta).get_amount();

        voter_info = get_voter(voter1, treasury_symbol);

        BOOST_REQUIRE_EQUAL(voter_info["staked"].as<asset>().get_amount(), staked_weight_amount);
    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( voting_methods, decide_tester ) try {
        
        
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

        base_tester::transfer(voter1, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter2, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter3, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(manager, decide_name, "5000.0000 TLOS", "", token_name);
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter1), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter2), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter3), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, manager), asset::from_string("5000.0000 TLOS"));
        
        new_treasury(manager, max_supply, name("public"));
        toggle(manager, treasury_symbol, name("stakeable"));

        reg_voter(voter1, treasury_symbol, {});
        reg_voter(voter2, treasury_symbol, {});
        reg_voter(voter3, treasury_symbol, {});

        mint(manager, voter1, asset::from_string("1000.00 GOO"), "init amount");
        mint(manager, voter2, asset::from_string("1000.00 GOO"), "init amount");
        mint(manager, voter3, asset::from_string("1000.00 GOO"), "init amount");
        
        BOOST_REQUIRE_EQUAL(get_voter(voter1, treasury_symbol)["liquid"].as<asset>(), asset::from_string("1000.00 GOO"));
        BOOST_REQUIRE_EQUAL(get_voter(voter2, treasury_symbol)["liquid"].as<asset>(), asset::from_string("1000.00 GOO"));
        BOOST_REQUIRE_EQUAL(get_voter(voter3, treasury_symbol)["liquid"].as<asset>(), asset::from_string("1000.00 GOO"));
        
        stake(voter1, asset::from_string("1000.00 GOO"));
        stake(voter2, asset::from_string("1000.00 GOO"));
        stake(voter3, asset::from_string("1000.00 GOO"));

        asset raw_vote_weight = get_voter(voter1, treasury_symbol)["staked"].as<asset>();

        BOOST_REQUIRE_EQUAL(get_voter(voter1, treasury_symbol)["staked"].as<asset>(), asset::from_string("1000.00 GOO"));
        BOOST_REQUIRE_EQUAL(get_voter(voter2, treasury_symbol)["staked"].as<asset>(), asset::from_string("1000.00 GOO"));
        BOOST_REQUIRE_EQUAL(get_voter(voter3, treasury_symbol)["staked"].as<asset>(), asset::from_string("1000.00 GOO"));

        name option1 = name("option1");
        name option2 = name("option2");

        //create a ballot for each voting method
        new_ballot(one_account_one_vote, category, voter1, treasury_symbol, one_account_one_vote, { option1, option2 });
        edit_min_max(voter1, one_account_one_vote, 2, 2);

        new_ballot(one_token_n_vote, category, voter1, treasury_symbol, one_token_n_vote, { option1, option2 });
        edit_min_max(voter1, one_token_n_vote, 2, 2);

        new_ballot(one_token_one_vote, category, voter1, treasury_symbol, one_token_one_vote, { option1, option2 });
        edit_min_max(voter1, one_token_one_vote, 2, 2);

        new_ballot(one_token_square_one_vote, category, voter1, treasury_symbol, one_token_square_one_vote, { option1, option2 });
        edit_min_max(voter1, one_token_square_one_vote, 2, 2);

        new_ballot(quadratic, category, voter1, treasury_symbol, quadratic, { option1, option2 });
        edit_min_max(voter1, quadratic, 2, 2);

        //one_account_one_vote weight testing
        open_voting(voter1, one_account_one_vote, get_current_time_point_sec() + 86400);
        produce_blocks();

        cast_vote(voter1, one_account_one_vote, { option1, option2 });
        fc::variant ballot_info = get_ballot(one_account_one_vote);
        map<name, asset> option_map = variant_to_map<name, asset>(ballot_info["options"]);

        //validate asset quantity per option
        //vote weight should be equal to 1 whole token undivided
        BOOST_REQUIRE_EQUAL(option_map[option1], one_acct_one_vote_calc(raw_vote_weight, treasury_symbol));
        BOOST_REQUIRE_EQUAL(option_map[option2], one_acct_one_vote_calc(raw_vote_weight, treasury_symbol));

        fc::variant vote_info = get_vote(one_account_one_vote, voter1);
        option_map = variant_to_map<name, asset>(vote_info["weighted_votes"]);
        BOOST_REQUIRE_EQUAL(option_map[option1], one_acct_one_vote_calc(raw_vote_weight, treasury_symbol));
        BOOST_REQUIRE_EQUAL(option_map[option2], one_acct_one_vote_calc(raw_vote_weight, treasury_symbol));
        

        //one_account_n_votes testing
        open_voting(voter1, one_token_n_vote, get_current_time_point_sec() + 86401);
        produce_blocks();

        cast_vote(voter1, one_token_n_vote, { option1, option2 });
        ballot_info = get_ballot(one_token_n_vote);
        option_map = variant_to_map<name, asset>(ballot_info["options"]);


        //validate asset quantity per option
        //vote weight should be equal to 1000.00 GOO per option, undivided
        BOOST_REQUIRE_EQUAL(option_map[option1], one_token_n_vote_calc(raw_vote_weight, treasury_symbol));
        BOOST_REQUIRE_EQUAL(option_map[option2], one_token_n_vote_calc(raw_vote_weight, treasury_symbol));

        vote_info = get_vote(one_token_n_vote, voter1);
        option_map = variant_to_map<name, asset>(vote_info["weighted_votes"]);
        BOOST_REQUIRE_EQUAL(option_map[option1], one_token_n_vote_calc(raw_vote_weight, treasury_symbol));
        BOOST_REQUIRE_EQUAL(option_map[option2], one_token_n_vote_calc(raw_vote_weight, treasury_symbol));



        //one_token_one_vote testing
        open_voting(voter1, one_token_one_vote, get_current_time_point_sec() + 86401);
        produce_blocks();

        cast_vote(voter1, one_token_one_vote, { option1, option2 });
        ballot_info = get_ballot(one_token_one_vote);
        option_map = variant_to_map<name, asset>(ballot_info["options"]);

        //validate asset quantity per option
        //vote weight should be equal to 1000.00 GOO per option, undivided
        BOOST_REQUIRE_EQUAL(option_map[option1], one_token_one_vote_calc(raw_vote_weight, treasury_symbol, 2));
        BOOST_REQUIRE_EQUAL(option_map[option2], one_token_one_vote_calc(raw_vote_weight, treasury_symbol, 2));

        vote_info = get_vote(one_token_one_vote, voter1);
        option_map = variant_to_map<name, asset>(vote_info["weighted_votes"]);
        BOOST_REQUIRE_EQUAL(option_map[option1], one_token_one_vote_calc(raw_vote_weight, treasury_symbol, 2));
        BOOST_REQUIRE_EQUAL(option_map[option2], one_token_one_vote_calc(raw_vote_weight, treasury_symbol, 2));



        //one_token_square_one_vote testing
        open_voting(voter1, one_token_square_one_vote, get_current_time_point_sec() + 86410);
        produce_blocks();

        cast_vote(voter1, one_token_square_one_vote, { option1, option2 });
        ballot_info = get_ballot(one_token_square_one_vote);
        option_map = variant_to_map<name, asset>(ballot_info["options"]);

        //validate asset quantity per option
        //vote weight should be equal to 1000.00 GOO per option, undivided
        BOOST_REQUIRE_EQUAL(option_map[option1], one_t_square_one_vote_calc(raw_vote_weight, treasury_symbol, 2));
        BOOST_REQUIRE_EQUAL(option_map[option2], one_t_square_one_vote_calc(raw_vote_weight, treasury_symbol, 2));

        vote_info = get_vote(one_token_square_one_vote, voter1);
        option_map = variant_to_map<name, asset>(vote_info["weighted_votes"]);
        BOOST_REQUIRE_EQUAL(option_map[option1], one_t_square_one_vote_calc(raw_vote_weight, treasury_symbol, 2));
        BOOST_REQUIRE_EQUAL(option_map[option2], one_t_square_one_vote_calc(raw_vote_weight, treasury_symbol, 2));


        //quadratic testing
        open_voting(voter1, quadratic, get_current_time_point_sec() + 86400 + 10);
        produce_blocks();

        cast_vote(voter1, quadratic, { option1, option2 });
        ballot_info = get_ballot(quadratic);
        option_map = variant_to_map<name, asset>(ballot_info["options"]);

        //validate asset quantity per option
        //vote weight should be equal to 1000.00 GOO per option, undivided
        BOOST_REQUIRE_EQUAL(option_map[option1], quadratic_calc(raw_vote_weight, treasury_symbol));
        BOOST_REQUIRE_EQUAL(option_map[option2], quadratic_calc(raw_vote_weight, treasury_symbol));

        vote_info = get_vote(quadratic, voter1);
        option_map = variant_to_map<name, asset>(vote_info["weighted_votes"]);
        BOOST_REQUIRE_EQUAL(option_map[option1], quadratic_calc(raw_vote_weight, treasury_symbol));
        BOOST_REQUIRE_EQUAL(option_map[option2], quadratic_calc(raw_vote_weight, treasury_symbol));
    } FC_LOG_AND_RETHROW()

    BOOST_FIXTURE_TEST_CASE( worker_basics, decide_tester ) try {
        
        //initialize
        asset max_supply = asset::from_string("1000000000.0000 VOTE");
        symbol treasury_symbol = max_supply.get_symbol();
        name ballot_name = name("ballot1");
        name category = name("proposal");
        name option1 = name("option1"), option2 = name("option2");
        name voting_method = name("1tokennvote");
        name voter1 = testa, voter2 = testb, voter3 = testc, worker = name("worker");

        //create worker account
        create_account_with_resources(worker, eosio_name, asset::from_string("1000.0000 TLOS"), false);

        //deposit funds into decide
        base_tester::transfer(voter1, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter2, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(voter3, decide_name, "3000.0000 TLOS", "", token_name);
        base_tester::transfer(eosio_name, decide_name, "10000.0000 TLOS", "", token_name);

        //assert table values
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter1), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter2), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, voter3), asset::from_string("3000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(decide_name, tlos_sym, eosio_name), asset::from_string("10000.0000 TLOS"));
        
        //create VOTE treasury
        new_treasury(eosio_name, max_supply, name("public"));
        produce_blocks();

        //register voter1
        reg_voter(voter1, treasury_symbol, { });
        produce_blocks();

        //check that cpu + net quantity = total staked in trail
        fc::variant user_resource_info = get_user_res(voter1);
        asset current_stake = user_resource_info["net_weight"].as<asset>() + user_resource_info["cpu_weight"].as<asset>();

        BOOST_REQUIRE_EQUAL(get_voter(voter1, treasury_symbol)["staked"].as<asset>(), tlos_to_vote(current_stake));
        
        asset stake_delta = asset::from_string("40.0000 TLOS");
        asset initial_stake = asset::from_string("20.0000 TLOS");

        //open ballot
        new_ballot(ballot_name, category, voter1, treasury_symbol, voting_method, { option1, option2 });
        edit_min_max(voter1, ballot_name, 2, 2);
        toggle_bal(voter1, ballot_name, name("votestake"));
        open_voting(voter1, ballot_name, get_current_time_point_sec() + 86410);
        produce_blocks();
        BOOST_REQUIRE(!get_ballot(ballot_name).is_null());

        cast_vote(voter1, ballot_name, { option1, option2 });
        produce_blocks();

        //checking initial voting quantity
        fc::variant ballot_info = get_ballot(ballot_name);
        map<name, asset> option_map = variant_to_map<name, asset>(ballot_info["options"]);
        BOOST_REQUIRE_EQUAL(option_map[option1], tlos_to_vote(initial_stake));
        BOOST_REQUIRE_EQUAL(option_map[option2], tlos_to_vote(initial_stake));

        // BOOST_REQUIRE_EXCEPTION(rebalance(worker, voter1, ballot_name, { worker }),
        //     eosio_assert_message_exception, eosio_assert_message_is( "vote is already balanced" ) 
        // );

        //change stake in both trail and eosio
        delegate_bw(voter1, voter1, stake_delta, stake_delta, false);

        //ballot quantities should be unchanged, requires rebalance
        ballot_info = get_ballot(ballot_name);
        option_map = variant_to_map<name, asset>(ballot_info["options"]);
        BOOST_REQUIRE_EQUAL(option_map[option1], tlos_to_vote(initial_stake));
        BOOST_REQUIRE_EQUAL(option_map[option2], tlos_to_vote(initial_stake));

        //vote quantities should be unchanged
        option_map = variant_to_map<name, asset>(get_vote(ballot_name, voter1)["weighted_votes"]);
        BOOST_REQUIRE_EQUAL(option_map[option1], tlos_to_vote(initial_stake));
        BOOST_REQUIRE_EQUAL(option_map[option2], tlos_to_vote(initial_stake));

        //validate that user stake and voter stake has changed
        user_resource_info = get_user_res(voter1);
        current_stake = user_resource_info["net_weight"].as<asset>() + user_resource_info["cpu_weight"].as<asset>();
        BOOST_REQUIRE_EQUAL(current_stake, stake_delta + stake_delta + initial_stake);

        BOOST_REQUIRE_EQUAL(get_voter(voter1, treasury_symbol)["staked"].as<asset>(), tlos_to_vote(initial_stake + stake_delta + stake_delta));

        //worker labor emplacement should be null, no work has been done
        BOOST_REQUIRE(get_labor(treasury_symbol, worker).is_null());

        auto validate_bucket = [&](uint32_t rebal_count, uint32_t clean_count, asset rebal_volume) {
            fc::variant bucket = get_labor_bucket(treasury_symbol, name("workers"));
            cout << endl << "bucket: " << bucket << endl;

            map<name, asset> claimable_volume = variant_to_map<name, asset>(bucket["claimable_volume"]);
            map<name, uint32_t> claimable_events = variant_to_map<name, uint32_t>(bucket["claimable_events"]);

            BOOST_REQUIRE_EQUAL(claimable_volume[name("rebalvolume")], rebal_volume);
            BOOST_REQUIRE_EQUAL(claimable_events[name("rebalcount")], rebal_count);
            BOOST_REQUIRE_EQUAL(claimable_events[name("cleancount")], clean_count);
            cout << endl << endl;
        };

        //rebalance votes and validate new quantity matches new stake
        rebalance(worker, voter1, ballot_name, { worker });
        produce_blocks();

        validate_bucket(0, 0, asset::from_string("0.0000 VOTE"));

        //ballot quantities should be updated to current stake in VOTE
        ballot_info = get_ballot(ballot_name);
        option_map = variant_to_map<name, asset>(ballot_info["options"]);
        BOOST_REQUIRE_EQUAL(option_map[option1], tlos_to_vote(current_stake));
        BOOST_REQUIRE_EQUAL(option_map[option2], tlos_to_vote(current_stake));

        //vote quantities should be changed to current stake in VOTE
        option_map = variant_to_map<name, asset>(get_vote(ballot_name, voter1)["weighted_votes"]);
        BOOST_REQUIRE_EQUAL(option_map[option1], tlos_to_vote(current_stake));
        BOOST_REQUIRE_EQUAL(option_map[option2], tlos_to_vote(current_stake));

        //get labor, should still be null, because rebalances are logged at the same time as cleaning
        BOOST_REQUIRE(get_labor(treasury_symbol, worker).is_null());
        // BOOST_REQUIRE_EXCEPTION(rebalance(worker, voter1, ballot_name, { worker }),
        //     eosio_assert_message_exception, eosio_assert_message_is( "vote is already balanced" )
        // );

        //end voting, kill del ballot, clean old votes, validate
        produce_block(fc::seconds(86420));
        produce_blocks();

        close_ballot(voter1, ballot_name, true);

        //claim payment for work done
        cleanup_vote(worker, voter1, ballot_name, worker);

        BOOST_REQUIRE(get_vote(ballot_name, voter1).is_null());

        fc::variant labor_info = get_labor(treasury_symbol, worker);
        BOOST_REQUIRE_EQUAL(labor_info["worker_name"].as<name>(), worker);

        map<name, asset> unclaimed_volume = variant_to_map<name, asset>(labor_info["unclaimed_volume"]);
        BOOST_REQUIRE_EQUAL(unclaimed_volume[name("rebalvolume")], tlos_to_vote(stake_delta + stake_delta));

        map<name, uint32_t> unclaimed_events = variant_to_map<name, uint32_t>(labor_info["unclaimed_events"]);
        BOOST_REQUIRE_EQUAL(unclaimed_events[name("cleancount")], 1);
        BOOST_REQUIRE_EQUAL(unclaimed_events[name("rebalcount")], 1);

        add_funds(eosio_name, eosio_name, treasury_symbol, name("workers"), asset::from_string("1000.0000 TLOS"));
        edit_pay_rate(eosio_name, name("workers"), treasury_symbol, 86400, asset::from_string("500.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(get_payroll(treasury_symbol, name("workers"))["payroll_funds"].as<asset>(), asset::from_string("1000.0000 TLOS"));

        fc::variant payroll = get_payroll(treasury_symbol, name("workers"));

        BOOST_REQUIRE_EQUAL(payroll["payroll_funds"].as<asset>(), asset::from_string("1000.0000 TLOS"));
        BOOST_REQUIRE_EQUAL(payroll["period_length"].as<uint32_t>(), 86400);
        BOOST_REQUIRE_EQUAL(payroll["per_period"].as<asset>(), asset::from_string("500.0000 TLOS"));

        produce_blocks();

        auto vote_cast_rebalance = [&](name voter, asset delta, name worker, symbol treasury_symbol, 
            name ballot_name, vector<name> options, bool redelegate = true, fc::optional<name> referrer = {}) {
            
            if(get_voter(voter,treasury_symbol).is_null()) {
                reg_voter(voter, treasury_symbol, {});
            }

            cast_vote(voter, ballot_name, options);

            if(redelegate) {
                delegate_bw(voter, voter, delta, delta, false);

                rebalance(worker, voter, ballot_name, { worker });
            }

        };
        
        name worker1 = name("worker1"), worker2 = name("worker2"), worker3 = name("worker3");
        name voter4 = name("voter4"), voter5 = name("votera"), voter6 = name("voterb");
        ballot_name = name("ballot12");
        
        create_accounts_with_resources({ worker1, worker2, worker3, voter4, voter5, voter6 });
        base_tester::transfer(eosio_name, voter4, "10000.0000 TLOS", "", token_name);
        base_tester::transfer(eosio_name, voter5, "10000.0000 TLOS", "", token_name);
        base_tester::transfer(eosio_name, voter6, "10000.0000 TLOS", "", token_name);
        
        new_ballot(ballot_name, category, voter1, treasury_symbol, voting_method, { option1, option2 });
        edit_min_max(voter1, ballot_name, 1, 2);
        toggle_bal(voter1, ballot_name, name("votestake"));
        open_voting(voter1, ballot_name, get_current_time_point_sec() + 86401);

        vote_cast_rebalance(voter1, asset::from_string("10.0000 TLOS"), worker, treasury_symbol, ballot_name, { option1, option2 });
        vote_cast_rebalance(voter2, asset::from_string("10.0000 TLOS"), worker, treasury_symbol, ballot_name, { option1 });

        produce_blocks();

        vote_cast_rebalance(voter3, asset::from_string("10.0000 TLOS"), worker1, treasury_symbol, ballot_name, { option1, option2 });
        vote_cast_rebalance(voter4, asset::from_string("10.0000 TLOS"), worker1, treasury_symbol, ballot_name, { option1 });

        produce_blocks();

        vote_cast_rebalance(voter5, asset::from_string("15.0000 TLOS"), worker2, treasury_symbol, ballot_name, { option1 });

        produce_blocks();

        vote_cast_rebalance(voter6, asset::from_string("10.0000 TLOS"), worker3, treasury_symbol, ballot_name, { option1 });

        produce_blocks();
        produce_block(fc::seconds(86401));
        produce_blocks();

        cleanup_vote(worker, voter1, ballot_name, { worker });
        cleanup_vote(worker, voter2, ballot_name, { worker });

        cleanup_vote(worker1, voter3, ballot_name, { worker1 });
        cleanup_vote(worker1, voter4, ballot_name, { worker1 });

        //TODO: voter5 was rebalanced by worker2, but worker1 cleans vote validate counts for each worker
        cleanup_vote(worker1, voter5, ballot_name, { worker1 });

        cleanup_vote(worker3, voter6, ballot_name, { worker3 });

        produce_blocks();
        produce_block(fc::days(2));
        produce_blocks();

        auto validate_labor = [&](const auto& worker, asset rebal_volume, uint32_t rebal_count, uint32_t clean_count) {
            fc::variant labor = get_labor(treasury_symbol, worker);
            cout << endl << "labor: " << labor << endl;
            map<name, asset> unclaimed_volume = variant_to_map<name, asset>(labor["unclaimed_volume"]);
            map<name, uint32_t> unclaimed_events = variant_to_map<name, uint32_t>(labor["unclaimed_events"]);

            BOOST_REQUIRE_EQUAL(unclaimed_volume[name("rebalvolume")], rebal_volume);
            BOOST_REQUIRE_EQUAL(unclaimed_events[name("rebalcount")], rebal_count);
            BOOST_REQUIRE_EQUAL(unclaimed_events[name("cleancount")], clean_count);
            cout << endl << endl;
        };

        validate_labor(worker, asset::from_string("120.0000 VOTE"), 3, 3);
        validate_labor(worker1, asset::from_string("40.0000 VOTE"), 2, 3);
        validate_labor(worker2, asset::from_string("30.0000 VOTE"), 1, 0);
        validate_labor(worker3, asset::from_string("20.0000 VOTE"), 1, 1);

        auto claim_validate = [&](const auto& worker) -> asset {
            asset worker_init_balance = base_tester::get_currency_balance(decide_name, tlos_sym, worker);
            cout << "initial balance: " << worker_init_balance << endl;
            
            fc::variant init_labor = get_labor(treasury_symbol, worker);
            fc::variant init_bucket = get_labor_bucket(treasury_symbol, name("workers"));

            map<name, asset> unclaimed_volume = variant_to_map<name, asset>(init_labor["unclaimed_volume"]);
            map<name, uint32_t> unclaimed_events = variant_to_map<name, uint32_t>(init_labor["unclaimed_events"]);

            map<name, asset> claimable_volume = variant_to_map<name, asset>(init_bucket["claimable_volume"]);
            map<name, uint32_t> claimable_events = variant_to_map<name, uint32_t>(init_bucket["claimable_events"]);

            asset pay_out = get_worker_claim(worker, treasury_symbol);
            cout << "pay_out balance: " << pay_out << endl;

            claim_payment(worker, treasury_symbol);
            BOOST_REQUIRE(get_labor(treasury_symbol, worker).is_null());

            asset current_balance = base_tester::get_currency_balance(decide_name, tlos_sym, worker);
            cout << "current balance: " << current_balance << endl;

            BOOST_REQUIRE_EQUAL(current_balance, worker_init_balance + pay_out);

            validate_bucket( 
                claimable_events[name("rebalcount")] - unclaimed_events[name("rebalcount")], 
                claimable_events[name("cleancount")] - unclaimed_events[name("cleancount")],
                claimable_volume[name("rebalvolume")] - unclaimed_volume[name("rebalvolume")]    
            );

            return pay_out;
        };

        auto forfeit_validate = [&](const auto& worker) {
            asset worker_init_balance = base_tester::get_currency_balance(decide_name, tlos_sym, worker);
            cout << "initial balance: " << worker_init_balance << endl;

            asset pay_out = get_worker_claim(worker, treasury_symbol);
            cout << "pay_out balance: " << pay_out << endl;

            fc::variant init_labor = get_labor(treasury_symbol, worker);
            fc::variant init_bucket = get_labor_bucket(treasury_symbol, name("workers"));

            map<name, asset> unclaimed_volume = variant_to_map<name, asset>(init_labor["unclaimed_volume"]);
            map<name, uint32_t> unclaimed_events = variant_to_map<name, uint32_t>(init_labor["unclaimed_events"]);

            map<name, asset> claimable_volume = variant_to_map<name, asset>(init_bucket["claimable_volume"]);
            map<name, uint32_t> claimable_events = variant_to_map<name, uint32_t>(init_bucket["claimable_events"]);

            forfeit_work(worker, treasury_symbol);
            BOOST_REQUIRE(get_labor(treasury_symbol, worker).is_null());

            asset current_balance = base_tester::get_currency_balance(decide_name, tlos_sym, worker);
            cout << "current balance: " << current_balance << endl << endl;

            BOOST_REQUIRE_EQUAL(current_balance, worker_init_balance);
            validate_bucket( 
                claimable_events[name("rebalcount")] - unclaimed_events[name("rebalcount")], 
                claimable_events[name("cleancount")] - unclaimed_events[name("cleancount")],
                claimable_volume[name("rebalvolume")] - unclaimed_volume[name("rebalvolume")]    
            );
        };

        asset worker_pay = claim_validate(worker);
        asset worker1_pay = claim_validate(worker1);
        asset worker2_pay = claim_validate(worker2);
        forfeit_validate(worker3);

        validate_bucket(0, 0, asset::from_string("0.0000 VOTE"));
        
        payroll = get_payroll(max_supply.get_symbol(), name("workers"));

        BOOST_REQUIRE_EQUAL(payroll["claimable_pay"].as<asset>(), asset::from_string("1000.0000 TLOS") - worker_pay - worker1_pay - worker2_pay);

    } FC_LOG_AND_RETHROW()
    
BOOST_AUTO_TEST_SUITE_END()