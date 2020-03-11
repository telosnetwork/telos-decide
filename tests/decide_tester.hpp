#pragma once

#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <fstream>

#include <boost/range/algorithm/find_if.hpp>

#include "contracts.hpp"

using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo = fc::mutable_variant_object;

namespace decidetesting {

    namespace  testing {

        class decide_tester : public tester {

            public:

            //system accounts
            const name eosio_name = name("eosio");
            const name token_name = name("eosio.token");
            const name rex_name = name("eosio.rex");
            const name ram_name = name("eosio.ram");
            const name ramfee_name = name("eosio.ramfee");
            const name stake_name = name("eosio.stake");
            const name bpay_name = name("eosio.bpay");
            const name vpay_name = name("eosio.vpay");
            const name names_name = name("eosio.names");

            //decide accounts
            const name decide_name = name("telos.decide");

            //test accounts
            const name testa = name("testaccounta");
            const name testb = name("testaccountb"); 
            const name testc = name("testaccountc");

            //legacy accounts
            const name trail_name = name("eosio.trail");

            //TABLE NAMEs
            const name config_tname = name("config");
            const name treasury_tname = name("treasuries");
            const name payroll_tname = name("payrolls");
            const name laborbucket_tname = name("laborbuckets");
            const name labors_tname = name("labors");
            const name ballots_tname = name("ballots");
            const name votes_tname = name("votes");
            const name voters_tname = name("voters");
            const name delegates_tname = name("delegates");
            const name committees_tname = name("committees");
            const name archivals_tname = name("archivals");
            const name featured_tname = name("featured");
            const name accounts_tname = name("accounts");
            
            //SYMBOLs
            const symbol tlos_sym = symbol(4, "TLOS");
            const symbol vote_sym = symbol(4, "VOTE");

            //ABI SERIALIZERs
            abi_serializer decide_abi_ser;
            abi_serializer token_abi_ser;
            abi_serializer sys_abi_ser;

            //======================== setup actions =======================

            enum class setup_mode {
                none,
                basic,
                full
            };

            decide_tester(setup_mode mode = setup_mode::full) {

                //create contract accounts
                create_accounts({ token_name, decide_name, rex_name, ram_name, ramfee_name, stake_name, bpay_name, vpay_name, names_name, trail_name });

                //setup system and token contracts
                setup_token_contract();
                setup_sys_contract();

                //initialize
                asset ram_amount = asset::from_string("400.0000 TLOS");
                asset liquid_amount = asset::from_string("10000.0000 TLOS");

                //create test accounts
                create_account_with_resources(testa, eosio_name, ram_amount, false);
                create_account_with_resources(testb, eosio_name, ram_amount, false);
                create_account_with_resources(testc, eosio_name, ram_amount, false);

                //fund test accounts
                base_tester::transfer(eosio_name, testa, "10000.0000 TLOS", "initial funds", token_name);
                base_tester::transfer(eosio_name, testb, "10000.0000 TLOS", "initial funds", token_name);
                base_tester::transfer(eosio_name, testc, "10000.0000 TLOS", "initial funds", token_name);

                //assert token balances
                BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, testa), liquid_amount);
                BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, testb), liquid_amount);
                BOOST_REQUIRE_EQUAL(base_tester::get_currency_balance(token_name, tlos_sym, testc), liquid_amount);

                
                if(mode == setup_mode::none) return; 

                //set code and abi
                set_code( decide_name, contracts::decide_wasm());
                set_abi( decide_name, contracts::decide_abi().data() );
                {
                    const auto& accnt = control->db().get<account_object,by_name>( decide_name );
                    abi_def abi;
                    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
                    decide_abi_ser.set_abi(abi, abi_serializer_max_time);
                }
                produce_blocks();
                
                if(mode == setup_mode::basic) return; 

                //initialize
                string decide_version = "v2.0.0";

                //init decide contract
                decide_init(decide_version);
                produce_blocks();

                cout << "Decide Full Setup Complete" << endl;
                
                if(mode == setup_mode::full) return;
            }

            void setup_token_contract() {
                
                //set code and abi
                set_code( token_name, contracts::token_wasm());
                set_abi( token_name, contracts::token_abi().data() );
                {
                    const auto& accnt = control->db().get<account_object,by_name>( token_name );
                    abi_def abi;
                    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
                    token_abi_ser.set_abi(abi, abi_serializer_max_time);
                }
                produce_blocks();

                //create, issue, open
                create(eosio_name, asset::from_string("1000000000.0000 TLOS"));
                issue(eosio_name, eosio_name, asset::from_string("1000000.0000 TLOS"), "");
                open(rex_name, tlos_sym, rex_name);
                produce_blocks();

            }

            void setup_sys_contract() {
                
                //set code and abi
                set_code( eosio_name, contracts::sys_wasm());
                set_abi( eosio_name, contracts::sys_abi().data() );
                {
                    const auto& accnt = control->db().get<account_object,by_name>( eosio_name );
                    abi_def abi;
                    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
                    sys_abi_ser.set_abi(abi, abi_serializer_max_time);
                }
                init(0, tlos_sym);
                produce_blocks();
                produce_block(fc::minutes(10));
                produce_blocks();
                produce_blocks(1000);

            }

            //======================== system/token actions ========================

            transaction_trace_ptr create_account_with_resources( account_name a, account_name creator, asset ramfunds, bool multisig,
                                                        asset net = asset::from_string("10.0000 TLOS"), asset cpu = asset::from_string("10.0000 TLOS") ) {
                signed_transaction trx;
                set_transaction_headers(trx);

                authority owner_auth;
                if (multisig) {
                    // multisig between account's owner key and creators active permission
                    owner_auth = authority(2, {key_weight{get_public_key( a, "owner" ), 1}}, {permission_level_weight{{creator, config::active_name}, 1}});
                } else {
                    owner_auth =  authority( get_public_key( a, "owner" ) );
                }

                trx.actions.emplace_back( vector<permission_level>{{creator,config::active_name}},
                                            newaccount{
                                            .creator  = creator,
                                            .name     = a,
                                            .owner    = owner_auth,
                                            .active   = authority( get_public_key( a, "active" ) )
                                            });

                trx.actions.emplace_back( get_action( N(eosio), N(buyram), vector<permission_level>{{creator,config::active_name}},
                                                        mvo()
                                                        ("payer", creator)
                                                        ("receiver", a)
                                                        ("quant", ramfunds) )
                                        );

                trx.actions.emplace_back( get_action( N(eosio), N(delegatebw), vector<permission_level>{{creator,config::active_name}},
                                                        mvo()
                                                        ("from", creator)
                                                        ("receiver", a)
                                                        ("stake_net_quantity", net )
                                                        ("stake_cpu_quantity", cpu )
                                                        ("transfer", 1 )
                                                    )
                                            );

                set_transaction_headers(trx);
                trx.sign( get_private_key( creator, "active" ), control->get_chain_id()  );
                return push_transaction( trx );
            }

            void create_accounts_with_resources(vector<name> names) {
                for (const auto& name : names) {
                    create_account_with_resources(name, eosio_name, asset::from_string("100.0000 TLOS"), false);
                }
            }

            transaction_trace_ptr delegate_bw(name from, name receiver, asset stake_net_quantity, asset stake_cpu_quantity, bool transfer) {
                return push_action(eosio_name, name("delegatebw"), { from }, mvo()
                    ("from", from)
                    ("receiver", receiver)
                    ("stake_net_quantity", stake_net_quantity)
                    ("stake_cpu_quantity", stake_cpu_quantity)
                    ("transfer", transfer)
                );
            }

            transaction_trace_ptr undelegate_bw(name from, name receiver, asset unstake_net_quantity, asset unstake_cpu_quantity) {
                return push_action(eosio_name, name("undelegatebw"), { from }, mvo()
                    ("from", from)
                    ("receiver", receiver)
                    ("unstake_net_quantity", unstake_net_quantity)
                    ("unstake_cpu_quantity", unstake_cpu_quantity)
                );
            }

            transaction_trace_ptr open(name owner, symbol symbol, name ram_payer) {
                return push_action(token_name, name("open"), { owner }, mvo()
                    ("owner", owner)
                    ("symbol", symbol)
                    ("ram_payer", ram_payer)
                );
            }

            transaction_trace_ptr create( account_name issuer, asset maximum_supply ) {
                return push_action(token_name, name("create"), { token_name }, mvo()
                    ("issuer", issuer)
                    ("maximum_supply", maximum_supply)
                );
            }

            transaction_trace_ptr issue(name issuer, name to, asset quantity, string memo) {
                return push_action(token_name, name("issue"), { issuer }, mvo()
                    ("to", to)
                    ("quantity", quantity)
                    ("memo", memo)
                );
            }

            transaction_trace_ptr init(uint32_t version, symbol core_symbol) {
                return push_action(eosio_name, name("init"), { eosio_name }, mvo()
                    ("version", version)
                    ("core", core_symbol)
                );
            }

            //======================== admin actions ========================

            //sets new config singleton
            transaction_trace_ptr decide_init(string app_version) {
                signed_transaction trx;
                vector<permission_level> permissions { { decide_name, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("init"), permissions, 
                    mvo()
                        ("app_version", app_version)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(decide_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            transaction_trace_ptr decide_setversion(string new_app_version) {
                signed_transaction trx;
                vector<permission_level> permissions { { decide_name, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("setversion"), permissions, 
                    mvo()
                        ("new_app_version", new_app_version)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(decide_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //updates fee amount
            transaction_trace_ptr update_fee(name fee_name, asset fee_amount) {
                signed_transaction trx;
                vector<permission_level> permissions { { decide_name, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("updatefee"), permissions, 
                    mvo()
                        ("fee_name", fee_name)
                        ("fee_amount", fee_amount)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(decide_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //updates time length
            transaction_trace_ptr update_time(name time_name, uint32_t length) {
                signed_transaction trx;
                vector<permission_level> permissions { { decide_name, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("updatetime"), permissions, 
                    mvo()
                        ("time_name", time_name)
                        ("length", length)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(decide_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //======================== treasury actions ========================

            //create a new treasury
            transaction_trace_ptr new_treasury(name manager, asset max_supply, name access) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("newtreasury"), permissions, 
                    mvo()
                        ("manager", manager)
                        ("max_supply", max_supply)
                        ("access", access)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            transaction_trace_ptr edit_trs_info(name manager, symbol treasury_symbol, string title, string description, string icon) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("edittrsinfo"), permissions, 
                    mvo()
                        ("treasury_symbol", treasury_symbol)
                        ("title", title)
                        ("description", description)
                        ("icon", icon)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //toggle a treasury setting
            transaction_trace_ptr toggle(name manager, symbol treasury_symbol, name setting_name) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("toggle"), permissions, 
                    mvo()
                        ("treasury_symbol", treasury_symbol)
                        ("setting_name", setting_name)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //mint new tokens to the recipient
            transaction_trace_ptr mint(name manager, name to, asset quantity, string memo) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("mint"), permissions, 
                    mvo()
                        ("to", to)
                        ("quantity", quantity)
                        ("memo", memo)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //transfer tokens
            transaction_trace_ptr transfer(name from, name to, asset quantity, string memo) {
                signed_transaction trx;
                vector<permission_level> permissions { { from, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("transfer"), permissions, 
                    mvo()
                        ("from", from)
                        ("to", to)
                        ("quantity", quantity)
                        ("memo", memo)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(from, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //burn tokens from manager balance
            transaction_trace_ptr burn(name manager, asset quantity, string memo) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("burn"), permissions, 
                    mvo()
                        ("quantity", quantity)
                        ("memo", memo)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //reclaim tokens from voter to the manager balance
            transaction_trace_ptr reclaim(name manager, name voter, asset quantity, string memo) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("reclaim"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("quantity", quantity)
                        ("memo", memo)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //change max supply
            transaction_trace_ptr mutate_max(name manager, asset new_max_supply, string memo) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("mutatemax"), permissions, 
                    mvo()
                        ("new_max_supply", new_max_supply)
                        ("memo", memo)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //set new unlock auth
            transaction_trace_ptr set_unlocker(name manager, symbol treasury_symbol, name new_unlock_acct, name new_unlock_auth) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("setunlocker"), permissions, 
                    mvo()
                        ("treasury_symbol", treasury_symbol)
                        ("new_unlock_acct", new_unlock_acct)
                        ("new_unlock_auth", new_unlock_auth)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //lock a treasury
            transaction_trace_ptr lock(name manager, symbol treasury_symbol) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("lock"), permissions, 
                    mvo()
                        ("treasury_symbol", treasury_symbol)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //unlock a treasury
            transaction_trace_ptr unlock(name unlock_acct, symbol treasury_symbol) {
                signed_transaction trx;
                vector<permission_level> permissions { { unlock_acct, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("unlock"), permissions, 
                    mvo()
                        ("treasury_symbol", treasury_symbol)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(unlock_acct, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //======================== payroll actions ========================

            //adds to specified payroll
            transaction_trace_ptr add_funds(name authorizer, name from, symbol treasury_symbol, name payroll_name, asset quantity) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("addfunds"), permissions, 
                    mvo()
                        ("treasury_symbol", treasury_symbol)
                        ("payroll_name", payroll_name)
                        ("from", from)
                        ("quantity", quantity)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //edit pay rate
            transaction_trace_ptr edit_pay_rate(name manager, name payroll_name, symbol treasury_symbol, uint32_t period_length, asset per_period) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("editpayrate"), permissions, 
                    mvo()
                        ("payroll_name", payroll_name)
                        ("treasury_symbol", treasury_symbol)
                        ("period_length", period_length)
                        ("per_period", per_period)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }
        
            //======================== ballot actions ========================
        
            //creates a new ballot
            transaction_trace_ptr new_ballot(name ballot_name, name category, name publisher,  
                symbol treasury_symbol, name voting_method, vector<name> initial_options) {

                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("newballot"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("category", category)
                        ("publisher", publisher)
                        ("treasury_symbol", treasury_symbol)
                        ("voting_method", voting_method)
                        ("initial_options", initial_options)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //edits ballots details
            transaction_trace_ptr edit_details(name publisher, name ballot_name, string title, string description, string content) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("editdetails"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("title", title)
                        ("description", description)
                        ("content", content)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //toggles ballot settings
            transaction_trace_ptr toggle_bal(name publisher, name ballot_name, name setting_name) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("togglebal"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("setting_name", setting_name)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //edits ballot min and max options
            transaction_trace_ptr edit_min_max(name publisher, name ballot_name, uint8_t new_min_options, uint8_t new_max_options) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("editminmax"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("new_min_options", new_min_options)
                        ("new_max_options", new_max_options)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //adds an option to a ballot
            transaction_trace_ptr add_option(name publisher, name ballot_name, name new_option_name) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("addoption"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("new_option_name", new_option_name)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //removes an option from a ballot
            transaction_trace_ptr rmv_option(name publisher, name ballot_name, name option_name) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("rmvoption"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("option_name", option_name)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //readies a ballot for voting
            transaction_trace_ptr open_voting(name publisher, name ballot_name, time_point_sec end_time) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("openvoting"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("end_time", end_time)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //cancels a ballot
            transaction_trace_ptr cancel_ballot(name publisher, name ballot_name, string memo) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("cancelballot"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("memo", memo)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //deletes an expired ballot
            transaction_trace_ptr delete_ballot(name publisher, name ballot_name) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("deleteballot"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //posts results from a light ballot before closing
            transaction_trace_ptr post_results(name publisher, name ballot_name, map<name, asset> light_results, uint32_t total_voters) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("postresults"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("light_results", light_results)
                        ("total_voters", total_voters)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //closes a ballot and post final results
            transaction_trace_ptr close_ballot(name publisher, name ballot_name, bool broadcast) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("closevoting"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("broadcast", broadcast)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //broadcast ballot results
            transaction_trace_ptr broadcast(name ballot_name, map<name, asset> final_results, uint32_t total_voters) {
                signed_transaction trx;
                vector<permission_level> permissions { { decide_name, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("broadcast"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("final_results", final_results)
                        ("total_voters", total_voters)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(decide_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //archives a ballot for a fee
            transaction_trace_ptr archive(name publisher, name ballot_name, time_point_sec archived_until) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("archive"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("archived_until", archived_until)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //unarchives a ballot after archival time has expired
            transaction_trace_ptr unarchive(name publisher, name ballot_name, bool force) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("unarchive"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("force", force)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }
        
            //======================== voter actions ========================

            //registers a new voter
            transaction_trace_ptr reg_voter(name voter, symbol treasury_symbol, fc::optional<name> referrer) {
                signed_transaction trx;
                vector<permission_level> permissions { { (referrer) ? *referrer : voter, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("regvoter"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("treasury_symbol", treasury_symbol)
                        ("referrer", referrer)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key((referrer) ? *referrer : voter, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //unregisters an existing voter
            transaction_trace_ptr unreg_voter(name voter, symbol treasury_symbol) {
                signed_transaction trx;
                vector<permission_level> permissions { { voter, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("unregvoter"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("treasury_symbol", treasury_symbol)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(voter, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //casts a vote on a ballot
            transaction_trace_ptr cast_vote(name voter, name ballot_name, vector<name> options) {
                signed_transaction trx;
                vector<permission_level> permissions { { voter, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("castvote"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("ballot_name", ballot_name)
                        ("options", options)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(voter, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            // ACTION unvote(name voter, name ballot_name, name option_to_unvote);

            //rollback all votes on a ballot
            transaction_trace_ptr unvote_all(name voter, name ballot_name) {
                signed_transaction trx;
                vector<permission_level> permissions { { voter, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("unvoteall"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("ballot_name", ballot_name)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(voter, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //stake tokens from balance to staked balance
            transaction_trace_ptr stake(name voter, asset quantity) {
                signed_transaction trx;
                vector<permission_level> permissions { { voter, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("stake"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("quantity", quantity)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(voter, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //unstakes tokens from staked balance to liquid balance
            transaction_trace_ptr unstake(name voter, asset quantity) {
                signed_transaction trx;
                vector<permission_level> permissions { { voter, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("unstake"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("quantity", quantity)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(voter, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //======================== worker actions ========================

            //unregisters an existing worker
            transaction_trace_ptr forfeit_work(name worker_name, symbol treasury_symbol) {
                signed_transaction trx;
                vector<permission_level> permissions { { worker_name, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("forfeitwork"), permissions, 
                    mvo()
                        ("worker_name", worker_name)
                        ("treasury_symbol", treasury_symbol)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(worker_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //pays a worker
            transaction_trace_ptr claim_payment(name worker_name, symbol treasury_symbol) {
                signed_transaction trx;
                vector<permission_level> permissions { { worker_name, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("claimpayment"), permissions, 
                    mvo()
                        ("worker_name", worker_name)
                        ("treasury_symbol", treasury_symbol)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(worker_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //rebalance an unbalanced vote
            transaction_trace_ptr rebalance(name authorizer, name voter, name ballot_name, fc::optional<name> worker) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("rebalance"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("ballot_name", ballot_name)
                        ("worker", worker)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //cleans up an expired vote
            transaction_trace_ptr cleanup_vote(name authorizer, name voter, name ballot_name, fc::optional<name> worker) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("cleanupvote"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("ballot_name", ballot_name)
                        ("worker", worker)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //withdraws TLOS balance to eosio.token
            transaction_trace_ptr withdraw(name voter, asset quantity) {
                signed_transaction trx;
                vector<permission_level> permissions { { voter, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("withdraw"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("quantity", quantity)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(voter, "active"), control->get_chain_id());
                return push_transaction( trx );
            }
        
            //======================== committee actions ========================

            //registers a new committee for a treasury
            transaction_trace_ptr reg_committee(name committee_name, string committee_title,
                symbol treasury_symbol, vector<name> initial_seats, name registree) {

                signed_transaction trx;
                vector<permission_level> permissions { { registree, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("regcommittee"), permissions, 
                    mvo()
                        ("committee_name", committee_name)
                        ("committee_title", committee_title)
                        ("treasury_symbol", treasury_symbol)
                        ("initial_seats", initial_seats)
                        ("registree", registree)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(registree, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //adds a committee seat
            transaction_trace_ptr add_seat(name authorizer, name committee_name, symbol treasury_symbol, name new_seat_name) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("addseat"), permissions, 
                    mvo()
                        ("committee_name", committee_name)
                        ("treasury_symbol", treasury_symbol)
                        ("new_seat_name", new_seat_name)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //removes a committee seat
            transaction_trace_ptr remove_seat(name authorizer, name committee_name, symbol treasury_symbol, name seat_name) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("removeseat"), permissions, 
                    mvo()
                        ("committee_name", committee_name)
                        ("treasury_symbol", treasury_symbol)
                        ("seat_name", seat_name)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //assigns a new member to a committee seat
            transaction_trace_ptr assign_seat(name authorizer, name committee_name, symbol treasury_symbol, name seat_name, name seat_holder, string memo) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("assignseat"), permissions, 
                    mvo()
                        ("committee_name", committee_name)
                        ("treasury_symbol", treasury_symbol)
                        ("seat_name", seat_name)
                        ("seat_holder", seat_holder)
                        ("memo", memo)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //sets updater account and auth
            transaction_trace_ptr set_updater(vector<permission_level> permissions, name committee_name, symbol treasury_symbol, name updater_account, name updater_auth) {
                signed_transaction trx;
                trx.actions.emplace_back(get_action(decide_name, name("setupdater"), permissions, 
                    mvo()
                        ("committee_name", committee_name)
                        ("treasury_symbol", treasury_symbol)
                        ("updater_account", updater_account)
                        ("updater_auth", updater_auth)
                ));
                set_transaction_headers( trx );
                for(const auto& permission : permissions) {
                    trx.sign(get_private_key(permission.actor, "active"), control->get_chain_id());
                }
                
                return push_transaction( trx );
            }

            //deletes a committee
            transaction_trace_ptr del_committee(name authorizer, name committee_name, symbol treasury_symbol, string memo) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(decide_name, name("delcommittee"), permissions, 
                    mvo()
                        ("committee_name", committee_name)
                        ("treasury_symbol", treasury_symbol)
                        ("memo", memo)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }


            //======================== row getters  ========================

            fc::variant get_config() { 
                vector<char> data = get_row_by_account(decide_name, decide_name, config_tname, config_tname);
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("config", data, abi_serializer_max_time);
            }

            fc::variant get_treasury(symbol treasury_symbol) {
                vector<char> data = get_row_by_account(decide_name, decide_name, treasury_tname, treasury_symbol.to_symbol_code());
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("treasury", data, abi_serializer_max_time);
            }

            fc::variant get_payroll(symbol treasury_symbol, name payroll_name) {
                vector<char> data = get_row_by_account(decide_name, treasury_symbol.to_symbol_code(), payroll_tname, payroll_name);
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("payroll", data, abi_serializer_max_time);
            }

            fc::variant get_labor_bucket(symbol treasury_symbol, name payroll_name) {
                vector<char> data = get_row_by_account(decide_name, treasury_symbol.to_symbol_code(), laborbucket_tname, payroll_name);
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("labor_bucket", data, abi_serializer_max_time);
            }

            fc::variant get_labor(symbol treasury_symbol, name worker_name) {
                vector<char> data = get_row_by_account(decide_name, treasury_symbol.to_symbol_code(), labors_tname, worker_name);
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("labor", data, abi_serializer_max_time);
            }

            fc::variant get_ballot(name ballot_name) {
                vector<char> data = get_row_by_account(decide_name, decide_name, ballots_tname, ballot_name);
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("ballot", data, abi_serializer_max_time);
            }

            fc::variant get_vote(name ballot_name, name voter) {
                vector<char> data = get_row_by_account(decide_name, ballot_name, votes_tname, voter);
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("vote", data, abi_serializer_max_time);
            }

            fc::variant get_voter(name voter, symbol vote_symbol) {
                vector<char> data = get_row_by_account(decide_name, voter, voters_tname, vote_symbol.to_symbol_code());
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("voter", data, abi_serializer_max_time);
            }

            fc::variant get_delegate(symbol treasury_symbol, name delegate_name) {
                vector<char> data = get_row_by_account(decide_name, treasury_symbol.to_symbol_code(), delegates_tname, delegate_name);
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("delegate", data, abi_serializer_max_time);
            }

            fc::variant get_committee(symbol treasury_symbol, name commitee_name) {
                vector<char> data = get_row_by_account(decide_name, treasury_symbol.to_symbol_code(), committees_tname, commitee_name);
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("committee", data, abi_serializer_max_time);
            }

            fc::variant get_archival(name ballot_name) {
                vector<char> data = get_row_by_account(decide_name, decide_name, archivals_tname, ballot_name);
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("archival", data, abi_serializer_max_time);
            }

            fc::variant get_featured_ballot(symbol treasury_symbol, name ballot_name) {
                vector<char> data = get_row_by_account(decide_name, treasury_symbol.to_symbol_code(), featured_tname, ballot_name);
                return data.empty() ? fc::variant() : decide_abi_ser.binary_to_variant("featured_ballot", data, abi_serializer_max_time);
            }

            //======================== system getters =======================

            fc::variant get_user_res(name account) {
                vector<char> data = get_row_by_account(eosio_name, account, name("delband"), account);
                return data.empty() ? fc::variant() : sys_abi_ser.binary_to_variant("delegated_bandwidth", data, abi_serializer_max_time);
            }

            //======================== helper actions =======================

            template<typename T, typename U>
            map<T, U> variant_to_map(fc::variant input) {
                vector<fc::variant> variants = input.as<vector<fc::variant>>();
                map<T, U> output;
                for(const auto& v : variants) {
                    output.emplace(v["key"].as<T>(), v["value"].as<U>());
                }
                return output;
            }

            template<typename T, typename U>
            void validate_map(map<T, U> map_to_validate, T key, U validation_value) {
                BOOST_REQUIRE_EQUAL(map_to_validate.count(key), 1);
                auto itr = map_to_validate.find(key);
                BOOST_REQUIRE_EQUAL(key, itr->first);
                BOOST_REQUIRE_EQUAL(validation_value, itr->second);
            }

            template<typename T, typename U>
            vector<fc::variant> map_to_vector(map<T, U> input) {
                vector<fc::variant> output;
                for(auto i = input.begin(); i != input.end(); i++) {
                    output.emplace_back(mvo()
                        ("key", i->first)
                        ("value", i->second)
                    );
                }
                return output;
            }

            void validate_voter(name voter, symbol treasury_symbol, fc::variant validation_info) {
                fc::variant voter_info = get_voter(voter, treasury_symbol);
                BOOST_REQUIRE_EQUAL(voter_info["liquid"].as<asset>(), validation_info["liquid"].as<asset>());
                BOOST_REQUIRE_EQUAL(voter_info["staked"].as<asset>(), validation_info["staked"].as<asset>());
                
                // TODO: resolve off by one time checking
                // BOOST_REQUIRE_EQUAL(voter_info["staked_time"].as<string>(), validation_info["staked_time"].as<string>());

                BOOST_REQUIRE_EQUAL(voter_info["delegated"].as<asset>(), validation_info["delegated"].as<asset>());
                BOOST_REQUIRE_EQUAL(voter_info["delegated_to"].as<name>(), validation_info["delegated_to"].as<name>());
                // TODO: resolve off by one time checking
                // BOOST_REQUIRE_EQUAL(voter_info["delegation_time"].as<string>(), validation_info["delegation_time"].as<string>());
            }

            void validate_action_payer(transaction_trace_ptr trace, name account_name, name action_name, name payer) {
                auto itr = find_if(trace->action_traces.begin(), trace->action_traces.begin(), [&](const auto& a_trace) {
                    cout << "action name: " << a_trace.act.name << " action account: " << account_name << endl;
                    return a_trace.act.name == action_name && a_trace.act.account == account_name;
                });
                BOOST_REQUIRE(itr != trace->action_traces.end());
                BOOST_REQUIRE(validate_ram_payer(*itr, payer));
            }

            bool validate_ram_payer(action_trace trace, name payer) {
                auto& deltas = trace.account_ram_deltas;
                for(auto itr = deltas.begin(); itr != deltas.end(); itr++) {
                    if(itr->account == payer) {
                        return true;
                    }
                }
                return false;
            }

            asset tlos_to_vote(asset tlos_asset) {
                return asset(tlos_asset.get_amount(), vote_sym);
            }

            //======================== time helpers =======================

            uint64_t get_current_time() {
                return static_cast<uint64_t>( control->pending_block_time().time_since_epoch().count());
            }
            
            time_point get_current_time_point() { 
                return time_point{ microseconds{ static_cast<int64_t>( get_current_time() ) } };
            }

            time_point_sec get_current_time_point_sec() {
                return time_point_sec{ get_current_time_point() };
            }

            //======================== payroll helpers =======================

            asset get_worker_claim(name worker, symbol treasury_symbol, name payroll_name = name("workers")) {
                fc::variant labor = get_labor(treasury_symbol, worker);
                fc::variant labor_bucket = get_labor_bucket(treasury_symbol, payroll_name);
                fc::variant payroll = get_payroll(treasury_symbol, payroll_name);

                //calc payment
                time_point_sec last_claim_time = payroll["last_claim_time"].as<time_point_sec>();
                time_point_sec start_time = labor["start_time"].as<time_point_sec>();
                uint32_t period_length = payroll["period_length"].as<uint32_t>();
                uint32_t now = get_current_time_point_sec().sec_since_epoch();

                // cout << "now: " << get_current_time_point_sec().to_iso_string() << endl;

                // cout << "last_claim_time: " << last_claim_time.to_iso_string() << endl;
                // cout << "start_time: " << start_time.to_iso_string()  << endl;

                asset new_claimable_pay = payroll["claimable_pay"].as<asset>();
                asset payroll_funds = payroll["payroll_funds"].as<asset>();
                asset additional_pay = asset(0, payroll_funds.get_symbol());
                asset per_period = payroll["per_period"].as<asset>();

                map<name, asset> unclaimed_volume = variant_to_map<name, asset>(labor["unclaimed_volume"]);
                map<name, uint32_t> unclaimed_events = variant_to_map<name, uint32_t>(labor["unclaimed_events"]);

                map<name, asset> total_claimable_volume = variant_to_map<name, asset>(labor_bucket["claimable_volume"]);
                map<name, uint32_t> total_claimable_events = variant_to_map<name, uint32_t>(labor_bucket["claimable_events"]);
                
                // cout << "period_length: " << period_length << endl;

                if (start_time.sec_since_epoch() + 86400 > now) {
                    BOOST_REQUIRE(false);
                }

                if(last_claim_time.sec_since_epoch() + period_length < now) {
                    uint32_t new_periods = (now - last_claim_time.sec_since_epoch()) / period_length;
                    // cout << "new_periods: " << new_periods << endl;
                    
                    additional_pay = asset(int64_t(per_period.get_amount() * new_periods), tlos_sym);
                    // cout << "additional_pay: " << additional_pay << endl;

                    if (payroll_funds <= additional_pay) {
                        additional_pay = payroll_funds;
                    }

                    new_claimable_pay += additional_pay;
                    // cout << "new_claimable_pay: " << new_claimable_pay << endl;
                }
                
                // cout << "now sec: " << now << endl;
                // cout << "start_time secs: " << start_time.sec_since_epoch() << endl;
                double reduced_by = ( ( ( now - start_time.sec_since_epoch() ) / 86400 ) - 1 ) / double(100);

                // cout << "reduced_by: " << reduced_by << endl;
                
                // the percentage of total volume rebalanced by this labor
                double vol_share = double(unclaimed_volume[name("rebalvolume")].get_amount()) / 
                    double(total_claimable_volume[name("rebalvolume")].get_amount());

                // cout << "vol_share: " << vol_share << endl;

                // the percentage of total rebalances performed by this labor
                double count_share = double(unclaimed_events[name("rebalcount")]) / 
                    double(total_claimable_events[name("rebalcount")]);

                // cout << "count_share: " << count_share << endl;

                // the percentage of total cleanings performed by this ths labor
                double clean_share = double(unclaimed_events[name("cleancount")]) / 
                    double(total_claimable_events[name("cleancount")]);

                // cout << "clean_share: " << clean_share << endl;

                double total_share = (vol_share + count_share + clean_share) / double(3.0);
                // cout << "total_share: " << total_share << endl;

                asset payout = asset(int64_t(new_claimable_pay.get_amount() * total_share), payroll_funds.get_symbol());
                // cout << "pre reduced payout: " << payout << endl;
                
                if(reduced_by > double(0) && reduced_by <= double(1.0)) {
                    // cout << "reduction applied" << endl;
                    payout = asset(payout.get_amount() * (double(1.0) - reduced_by), payout.get_symbol());
                } else {
                    payout = asset(0, payout.get_symbol());
                }

                if (payout > new_claimable_pay) {
                    payout = new_claimable_pay;
                }

                // cout << "post reduction final payout for " << worker << ": " << payout << endl << endl << endl;

                return payout;
            }

            //======================== voting calculations =======================

            asset one_acct_one_vote_calc(asset raw_weight, symbol treasury_symbol) {
                return asset(int64_t(pow(10, log10(treasury_symbol.precision()))), treasury_symbol);
            }

            asset one_token_n_vote_calc(asset raw_weight, symbol treasury_symbol) {
                return asset(raw_weight.get_amount(), treasury_symbol);
            }

            asset one_token_one_vote_calc(asset raw_weight, symbol treasury_symbol, uint8_t selections) {
                return asset(uint64_t(raw_weight.get_amount() / selections), treasury_symbol);
            } 

            asset one_t_square_one_vote_calc(asset raw_weight, symbol treasury_symbol, uint8_t selections) {
                uint64_t vote_amount_per = raw_weight.get_amount() / selections;
                return asset(vote_amount_per * vote_amount_per, treasury_symbol);
            }

            asset quadratic_calc(asset raw_weight, symbol treasury_symbol) {
                return asset(sqrtl(raw_weight.get_amount()), treasury_symbol);
            }
            
        };

    }
    
}