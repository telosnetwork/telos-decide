// Trail is an on-chain voting platform for the Telos Blockchain Network offering
// a full suite of voting services for users and developers.
//
// @author Craig Branscom
// @contract trail
// @version v2.0.0-RFC1
// @copyright see LICENSE.txt

#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

#include "utility.hpp"

#include <cmath>

using namespace eosio;
using namespace std;

CONTRACT trail : public contract {

public:

    trail(name self, name code, datastream<const char*> ds);

    ~trail();

    //reserved symbols
    const symbol TLOS_SYM = symbol("TLOS", 4);
    const symbol VOTE_SYM = symbol("VOTE", 4);
    const symbol TRAIL_SYM = symbol("TRAIL", 0);

    //registry settings: transferable, burnable, reclaimable, stakeable, maxmutable

    //registry access: public, private, invite, membership?

    //ballot settings: lightballot, revotable, votestake

    //ballot statuses: setup, voting, closed, cancelled, archived

    //voting methods: 1acct1vote, 1tokennvote, 1token1vote, 1tsquare1v, quadratic, ranked

    //ballot categories: proposal, referendum, election, poll, leaderboard

    //======================== admin actions ========================

    //sets new config singleton
    ACTION setconfig(string trail_version, bool set_defaults);

    //updates fee amount
    ACTION updatefee(name fee_name, asset fee_amount);

    //updates time length
    ACTION updatetime(name time_name, uint32_t length);

    //======================== registry actions ========================

    //create a new token registry
    ACTION newregistry(name manager, asset max_supply, name access);

    //toggle a registry setting
    ACTION togglereg(symbol registry_symbol, name setting_name);

    //mint new tokens to the recipient
    ACTION mint(name to, asset quantity, string memo);

    //transfer tokens
    ACTION transfer(name from, name to, asset quantity, string memo);

    //burn tokens from manager balance
    ACTION burn(asset quantity, string memo);

    //reclaim tokens from voter
    ACTION reclaim(name voter, asset quantity, string memo);

    //change max supply
    ACTION mutatemax(asset new_max_supply, string memo);

    //set new unlock auth
    ACTION setunlocker(symbol registry_symbol, name new_unlock_acct, name new_unlock_auth);

    //lock a token registry
    ACTION lockreg(symbol registry_symbol);

    //unlock a token registry
    ACTION unlockreg(symbol registry_symbol);

    //adds to registry worker fund
    ACTION addtofund(symbol registry_symbol, name voter, asset quantity);

    //======================== ballot actions ========================

    //creates a new ballot
    ACTION newballot(name ballot_name, name category, name publisher,  
        symbol registry_symbol, name voting_method, vector<name> initial_options);

    //edits ballots details
    ACTION editdetails(name ballot_name, string title, string description, string ballot_info);

    //toggles ballot settings
    ACTION togglebal(name ballot_name, name setting_name);

    //edits ballot max options
    ACTION editmaxopts(name ballot_name, uint8_t new_max_options);

    //adds an option to a ballot
    ACTION addoption(name ballot_name, name new_option_name);

    //removes an option from a ballot
    ACTION rmvoption(name ballot_name, name option_name);

    //readies a ballot for voting
    ACTION readyballot(name ballot_name, time_point_sec end_time);

    //cancels a ballot
    ACTION cancelballot(name ballot_name, string memo);

    //deletes an expired ballot
    ACTION deleteballot(name ballot_name);

    //posts results from a light ballot before closing
    ACTION postresults(name ballot_name, map<name, asset> light_results, uint32_t total_voters);

    //closes a ballot and post final results
    ACTION closeballot(name ballot_name, bool broadcast);

    //broadcast ballot results
    ACTION bcastresults(name ballot_name, map<name, asset> final_results, uint32_t total_voters);

    //archives a ballot for a fee
    ACTION archive(name ballot_name, time_point_sec archived_until);

    //unarchives a ballot after archival time has expired
    ACTION unarchive(name ballot_name, bool force);

    //======================== voter actions ========================

    //registers a new voter
    ACTION regvoter(name voter, symbol registry_symbol, optional<name> referrer);

    //unregisters an existing voter
    ACTION unregvoter(name voter, symbol registry_symbol);

    //casts a vote on a ballot
    ACTION castvote(name voter, name ballot_name, vector<name> options);

    //TODO: unvotes a single option 
    // ACTION unvote(name voter, name ballot_name, name option_to_unvote);

    //rollback all votes on a ballot
    ACTION unvoteall(name voter, name ballot_name);

    //stake tokens from balance to staked balance
    ACTION stake(name voter, asset quantity);

    //unstakes tokens from staked balance to liquid balance
    ACTION unstake(name voter, asset quantity);

    //======================== worker actions ========================

    //registers a new worker
    ACTION regworker(name worker_name);

    //unregisters an existing worker
    ACTION unregworker(name worker_name);

    //pays a worker
    ACTION claimpayment(name worker_name, symbol registry_symbol);

    //rebalance an unbalaned vote
    ACTION rebalance(name voter, name ballot_name, optional<name> worker);

    //rebalance a number of unbalanced votes
    // ACTION bigrebalance(name voter, symbol registry_symbol, optional<uint16_t> count, optional<name> worker);

    //cleans up an expired vote
    ACTION cleanupvote(name voter, name ballot_name, optional<name> worker);

    //attempts to clean all expired votes, or up to a given count
    // ACTION cleanhouse(name voter, optional<uint16_t> count, optional<name> worker);

    //======================== committee actions ========================

    //registers a new committee for a token registry
    ACTION regcommittee(name committee_name, string committee_title,
        symbol registry_symbol, vector<name> initial_seats, name registree);

    //adds a committee seat
    ACTION addseat(name committee_name, symbol registry_symbol, name new_seat_name);

    //removes a committee seat
    ACTION removeseat(name committee_name, symbol registry_symbol, name seat_name);

    //assigns a new member to a committee seat
    ACTION assignseat(name committee_name, symbol registry_symbol, name seat_name, name seat_holder, string memo);

    //sets updater account and auth
    ACTION setupdater(name committee_name, symbol registry_symbol, name updater_account, name updater_auth);

    //deletes a committee
    ACTION delcommittee(name committee_name, symbol registry_symbol, string memo);

    //========== notification methods ==========

    //catches delegatebw from eosio
    [[eosio::on_notify("eosio::delegatebw")]]
    void catch_delegatebw(name from, name receiver, asset stake_net_quantity, asset stake_cpu_quantity, bool transfer);

    //catches undelegatebw from eosio
    [[eosio::on_notify("eosio::undelegatebw")]]
    void catch_undelegatebw(name from, name receiver, asset unstake_net_quantity, asset unstake_cpu_quantity);

    //catches TLOS transfers from eosio.token
    [[eosio::on_notify("eosio.token::transfer")]]
    void catch_transfer(name from, name to, asset quantity, string memo);

    //========== utility methods ==========

    //add quantity to liquid amount
    void add_liquid(name voter, asset quantity);

    //subtract quantity from liquid amount
    void sub_liquid(name voter, asset quantity);

    //add quantity to staked amount
    void add_stake(name voter, asset quantity);

    //subtract quantity from staked amount
    void sub_stake(name voter, asset quantity);

    //validates category name
    bool valid_category(name category);

    //validates voting method
    bool valid_voting_method(name voting_method);

    //validates access method
    bool valid_access_method(name access_method);

    //charges a fee to a TLOS balance
    void require_fee(name account_name, asset fee);

    //logs rebalance work
    void log_rebalance_work(name worker, symbol registry_symbol, asset volume, uint16_t count);

    //logs cleanup work
    void log_cleanup_work(name worker, symbol registry_symbol, uint16_t count);

    //syncs an exernal account balance with a linked voter balance
    void sync_external_account(name voter, symbol internal_symbol, symbol external_symbol);

    //calculates vote mapping
    map<name, asset> calc_vote_weights(symbol registry_symbol, name voting_method, 
    vector<name> selections,  asset raw_vote_weight);

    //======================== tables ========================

    //scope: singleton
    //ram: 
    TABLE config {
        string trail_version;
        map<name, asset> fees; //ballot, registry, archival
        map<name, uint32_t> times; //balcooldown, minballength
        //map<name, double> job_weights; //rebalvolume, rebalcount, cleancount
    };
    typedef singleton<name("config"), config> config_singleton;

    //scope: get_self().value
    //ram: 
    TABLE registry {
        asset supply; //current supply
        asset max_supply; //maximum supply

        uint32_t voters; //open token accounts with this registry
        name access; //public, private, invite, membership

        bool locked; //locks all settings
        name unlock_acct; //account name to unlock
        name unlock_auth; //authorization name to unlock

        name manager; //registry manager
        map<name, bool> settings; //setting_name -> on/off

        uint16_t open_ballots; //number of open ballots
        asset worker_funds; //bucket to pay workers

        asset rebalanced_volume; //total volume of rebalanced votes
        uint32_t rebalanced_count; //total count of rebalanced votes
        uint32_t cleaned_count; //total count of cleaned votes

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
        EOSLIB_SERIALIZE(registry, 
            (supply)(max_supply)
            (voters)(access)
            (locked)(unlock_acct)(unlock_auth)
            (manager)(settings)
            (open_ballots)(worker_funds)
            (rebalanced_volume)(rebalanced_count)(cleaned_count))
    };
    typedef multi_index<name("registries"), registry> registries_table;

    //scope: get_self().value
    //ram:
    TABLE ballot {
        name ballot_name;
        name category; //proposal, referendum, election, poll, leaderboard
        name publisher;
        name status; //setup, voting, closed, cancelled, archived

        string title; //markdown
        string description; //markdown
        string ballot_info; //typically IPFS link to content

        name voting_method; //1acct1vote, 1tokennvote, 1token1vote, 1tsquare1v, quadratic, ranked
        uint8_t max_options; //max options per voter
        map<name, asset> options; //option name -> total weighted votes

        symbol registry_symbol; //token registry used for counting votes
        uint32_t total_voters; //unique voters who have voted on ballot
        //asset total_raw_weight; 
        map<name, bool> settings; //setting name -> on/off

        uint32_t cleaned_count; //number of expired vote receipts cleaned
        
        time_point_sec begin_time; //time that voting begins
        time_point_sec end_time; //time that voting closes

        uint64_t primary_key() const { return ballot_name.value; }
        EOSLIB_SERIALIZE(ballot, 
            (ballot_name)(category)(publisher)(status)
            (title)(description)(ballot_info)
            (voting_method)(max_options)(options)
            (registry_symbol)(total_voters)(settings)
            (cleaned_count)
            (begin_time)(end_time))
    };
    typedef multi_index<name("ballots"), ballot> ballots_table;

    //scope: voter.value
    //ram: 
    TABLE vote {
        name ballot_name;
        symbol registry_symbol;
        asset raw_vote_weight;
        map<name, asset> weighted_votes;
        time_point_sec expiration;
        
        name worker;
        uint8_t rebalances;
        asset rebalance_volume;

        uint64_t primary_key() const { return ballot_name.value; }
        uint64_t by_symbol() const { return registry_symbol.code().raw(); }
        uint64_t by_exp() const { return static_cast<uint64_t>(expiration.utc_seconds); }
        EOSLIB_SERIALIZE(vote, 
            (ballot_name)(registry_symbol)(raw_vote_weight)(weighted_votes)(expiration)
            (worker)(rebalances)(rebalance_volume))
    };
    typedef multi_index<name("votes"), vote,
        indexed_by<name("bysymbol"), const_mem_fun<vote, uint64_t, &vote::by_symbol>>,
        indexed_by<name("byexp"), const_mem_fun<vote, uint64_t, &vote::by_exp>>
    > votes_table;

    //scope: voter.value
    //ram: 
    TABLE voter {
        asset liquid;
        asset staked;

        uint64_t primary_key() const { return liquid.symbol.code().raw(); }
        EOSLIB_SERIALIZE(voter, (liquid)(staked))
    };
    typedef multi_index<name("voters"), voter> voters_table;

    //scope: registry_symbol.code().raw()
    //ram: 
    TABLE committee {
        string committee_title;
        name committee_name;

        symbol registry_symbol;
        map<name, name> seats; //seat_name -> seat_holder (0 if empty)

        name updater_acct; //account name that can update committee members
        name updater_auth; //auth name that can update committee members

        uint64_t primary_key() const { return committee_name.value; }
        EOSLIB_SERIALIZE(committee, 
            (committee_title)(committee_name)
            (registry_symbol)(seats)
            (updater_acct)(updater_auth))
    };
    typedef multi_index<name("committees"), committee> committees_table;

    //scope: get_self().value
    //ram: 
    TABLE worker {
        name worker_name;
        name standing;
        time_point_sec last_payment;

        //by registry symbol
        map<symbol, asset> rebalance_volume;
        map<symbol, uint16_t> rebalance_count;
        map<symbol, uint16_t> clean_count;

        uint64_t primary_key() const { return worker_name.value; }
        EOSLIB_SERIALIZE(worker, 
            (worker_name)(standing)(last_payment)
            (rebalance_volume)(rebalance_count)(clean_count))
    };
    typedef multi_index<name("workers"), worker> workers_table;

    //scope: get_self().value
    //ram:
    TABLE archival {
        name ballot_name;
        time_point_sec archived_until;

        uint64_t primary_key() const { return ballot_name.value; }
        EOSLIB_SERIALIZE(archival,
            (ballot_name)(archived_until))
    };
    typedef multi_index<name("archivals"), archival> archivals_table;

    //scope: account_name.value
    //ram: 
    TABLE account {
        asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }
        EOSLIB_SERIALIZE(account, (balance))
    };
    typedef multi_index<name("accounts"), account> accounts_table;

};