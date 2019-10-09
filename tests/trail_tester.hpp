#pragma once

#include <eosio/testing/tester.hpp>
#include <eosio/chain/abi_serializer.hpp>
#include <eosio/chain/resource_limits.hpp>
#include <fstream>

#include "contracts.hpp"

using namespace eosio::chain;
using namespace eosio::testing;
using namespace fc;
using namespace std;

using mvo = fc::mutable_variant_object;

namespace trail {

    namespace  testing {

        class trail_tester : public tester {
        public:
            //ACCOUNT NAMEs
            const name trail_name = name("trailservice");
            const name testa = name("testaccounta");
            const name testb = name("testaccountb"); 
            const name testc = name("testaccountc");

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
            const symbol trail_sym = symbol(4, "TRAIL");

            //ABI SERIALIZERs
            abi_serializer abi_ser;

            enum class setup_mode {
                none,
                basic,
                full
            };

            trail_tester(setup_mode mode = setup_mode::full) {
                create_accounts({ trail_name, testa, testb, testc });
                
                if(mode == setup_mode::none) return; 

                set_code( trail_name, contracts::trail_wasm());
                set_abi( trail_name, contracts::trail_abi().data() );
                {
                    const auto& accnt = control->db().get<account_object,by_name>( trail_name );
                    abi_def abi;
                    BOOST_REQUIRE_EQUAL(abi_serializer::to_abi(accnt.abi, abi), true);
                    abi_ser.set_abi(abi, abi_serializer_max_time);
                }

                if(mode == setup_mode::basic) return; 

                
                if(mode == setup_mode::full) return; 
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


            //======================== admin actions ========================

            //sets new config singleton
            transaction_trace_ptr set_config(string trail_version, bool set_defaults) {
                signed_transaction trx;
                vector<permission_level> permissions { { trail_name, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("setconfig"), permissions, 
                    mvo()
                        ("trail_version", trail_version)
                        ("set_defaults", set_defaults)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(trail_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //updates fee amount
            transaction_trace_ptr update_fee(name fee_name, asset fee_amount) {
                signed_transaction trx;
                vector<permission_level> permissions { { trail_name, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("updatefee"), permissions, 
                    mvo()
                        ("fee_name", fee_name)
                        ("fee_amount", fee_amount)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(trail_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //updates time length
            transaction_trace_ptr update_time(name time_name, uint32_t length) {
                signed_transaction trx;
                vector<permission_level> permissions { { trail_name, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("updatetime"), permissions, 
                    mvo()
                        ("time_name", time_name)
                        ("length", length)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(trail_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //======================== treasury actions ========================

            //create a new treasury
            transaction_trace_ptr new_treasury(name manager, asset max_supply, name access) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("newtreasury"), permissions, 
                    mvo()
                        ("manager", manager)
                        ("max_supply", max_supply)
                        ("access", access)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(manager, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //toggle a treasury setting
            transaction_trace_ptr toggle(name manager, symbol treasury_symbol, name setting_name) {
                signed_transaction trx;
                vector<permission_level> permissions { { manager, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("toggle"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("mint"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("transfer"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("burn"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("reclaim"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("mutatemax"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("setunlocker"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("lock"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("unlock"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("addfunds"), permissions, 
                    mvo()
                        ("treasury_symbol", treasury_symbol)
                        ("payroll_name", payroll_name)
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
                trx.actions.emplace_back(get_action(trail_name, name("editpayrate"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("newballot"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("category", category)
                        ("publisher", publisher)
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
                trx.actions.emplace_back(get_action(trail_name, name("editdetails"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("togglebal"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("editminmax"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("addoption"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("rmvoption"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("option_name", option_name)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(publisher, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //readies a ballot for voting
            transaction_trace_ptr ready_ballot(name publisher, name ballot_name, time_point_sec end_time) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("readyballot"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("cancelballot"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("deleteballot"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("postresults"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("closeballot"), permissions, 
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
                vector<permission_level> permissions { { trail_name, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("broadcast"), permissions, 
                    mvo()
                        ("ballot_name", ballot_name)
                        ("final_results", final_results)
                        ("total_voters", total_voters)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(trail_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //archives a ballot for a fee
            transaction_trace_ptr archive(name publisher, name ballot_name, time_point_sec archived_until) {
                signed_transaction trx;
                vector<permission_level> permissions { { publisher, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("archive"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("unarchive"), permissions, 
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
            transaction_trace_ptr reg_voter(name voter, symbol treasury_symbol, std::optional<name> referrer) {
                signed_transaction trx;
                vector<permission_level> permissions { { voter, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("regvoter"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("treasury_symbol", treasury_symbol)
                        ("referrer", *referrer)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(voter, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //unregisters an existing voter
            transaction_trace_ptr unreg_voter(name voter, symbol treasury_symbol) {
                signed_transaction trx;
                vector<permission_level> permissions { { voter, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("unregvoter"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("castvote"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("ballot_name", ballot_name)
                        ("options", options)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(voter, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //TODO: unvotes a single option
            // ACTION unvote(name voter, name ballot_name, name option_to_unvote);

            //rollback all votes on a ballot
            transaction_trace_ptr unvote_all(name voter, name ballot_name) {
                signed_transaction trx;
                vector<permission_level> permissions { { voter, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("unvoteall"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("stake"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("unstake"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("forgeitwork"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("claimpayment"), permissions, 
                    mvo()
                        ("worker_name", worker_name)
                        ("treasury_symbol", treasury_symbol)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(worker_name, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //rebalance an unbalanced vote
            transaction_trace_ptr rebalance(name authorizer, name voter, name ballot_name, std::optional<name> worker) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("rebalance"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("ballot_name", ballot_name)
                        ("worker", *worker)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //cleans up an expired vote
            transaction_trace_ptr cleanup_vote(name authorizer, name voter, name ballot_name, std::optional<name> worker) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("cleanupvote"), permissions, 
                    mvo()
                        ("voter", voter)
                        ("ballot_name", ballot_name)
                        ("worker", *worker)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //withdraws TLOS balance to eosio.token
            transaction_trace_ptr withdraw(name voter, asset quantity) {
                signed_transaction trx;
                vector<permission_level> permissions { { voter, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("withdraw"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("regcomittee"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("addseat"), permissions, 
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
                trx.actions.emplace_back(get_action(trail_name, name("removeseat"), permissions, 
                    mvo()
                        ("committee_name", committee_name)
                        ("treasury_symbol", treasury_symbol)
                        ("new_seat_name", seat_name)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //assigns a new member to a committee seat
            transaction_trace_ptr assign_seat(name authorizer, name committee_name, symbol treasury_symbol, name seat_name, name seat_holder, string memo) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("assignseat"), permissions, 
                    mvo()
                        ("committee_name", committee_name)
                        ("treasury_symbol", treasury_symbol)
                        ("new_seat_name", seat_name)
                        ("seat_holder", seat_holder)
                        ("memo", memo)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //sets updater account and auth
            transaction_trace_ptr set_updater(name authorizer, name committee_name, symbol treasury_symbol, name updater_account, name updater_auth) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("setupdater"), permissions, 
                    mvo()
                        ("committee_name", committee_name)
                        ("treasury_symbol", treasury_symbol)
                        ("updater_account", updater_account)
                        ("updater_auth", updater_auth)
                ));
                set_transaction_headers( trx );
                trx.sign(get_private_key(authorizer, "active"), control->get_chain_id());
                return push_transaction( trx );
            }

            //deletes a committee
            transaction_trace_ptr del_committee(name authorizer, name committee_name, symbol treasury_symbol, string memo) {
                signed_transaction trx;
                vector<permission_level> permissions { { authorizer, name("active") } };
                trx.actions.emplace_back(get_action(trail_name, name("delcommittee"), permissions, 
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
                vector<char> data = get_row_by_account(trail_name, trail_name, config_tname, config_tname);
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("config", data, abi_serializer_max_time);
            }

            fc::variant get_treasury(symbol treasury_symbol) {
                vector<char> data = get_row_by_account(trail_name, trail_name, treasury_tname, treasury_symbol.to_symbol_code());
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("treasury", data, abi_serializer_max_time);
            }

            fc::variant get_payroll(symbol treasury_symbol, name payroll_name) {
                vector<char> data = get_row_by_account(trail_name, treasury_symbol.to_symbol_code(), payroll_tname, payroll_name);
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("payroll", data, abi_serializer_max_time);
            }

            fc::variant get_labor_bucket(symbol treasury_symbol, name payroll_name) {
                vector<char> data = get_row_by_account(trail_name, treasury_symbol.to_symbol_code(), laborbucket_tname, payroll_name);
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("labor_bucket", data, abi_serializer_max_time);
            }

            fc::variant get_labor(symbol treasury_symbol, name worker_name) {
                vector<char> data = get_row_by_account(trail_name, treasury_symbol.to_symbol_code(), labors_tname, worker_name);
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("labor", data, abi_serializer_max_time);
            }

            fc::variant get_ballot(name ballot_name) {
                vector<char> data = get_row_by_account(trail_name, trail_name, ballots_tname, ballot_name);
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("ballot", data, abi_serializer_max_time);
            }

            fc::variant get_vote(name ballot_name, name voter) {
                vector<char> data = get_row_by_account(trail_name, ballot_name, votes_tname, voter);
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("vote", data, abi_serializer_max_time);
            }

            fc::variant get_voter(name voter, symbol vote_symbol) {
                vector<char> data = get_row_by_account(trail_name, voter, voters_tname, vote_symbol.to_symbol_code());
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("voter", data, abi_serializer_max_time);
            }

            fc::variant get_delegate(symbol treasury_symbol, name delegate_name) {
                vector<char> data = get_row_by_account(trail_name, treasury_symbol.to_symbol_code(), delegates_tname, delegate_name);
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("delegate", data, abi_serializer_max_time);
            }

            fc::variant get_committee(symbol treasury_symbol, name commitee_name) {
                vector<char> data = get_row_by_account(trail_name, treasury_symbol.to_symbol_code(), committees_tname, commitee_name);
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("comittee", data, abi_serializer_max_time);
            }

            fc::variant get_archival(name ballot_name) {
                vector<char> data = get_row_by_account(trail_name, trail_name, archivals_tname, ballot_name);
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("archival", data, abi_serializer_max_time);
            }

            fc::variant get_featured_ballot(symbol treasury_symbol, name ballot_name) {
                vector<char> data = get_row_by_account(trail_name, treasury_symbol.to_symbol_code(), featured_tname, ballot_name);
                return data.empty() ? fc::variant() : abi_ser.binary_to_variant("featured_ballot", data, abi_serializer_max_time);
            }
        };

    }
    
}