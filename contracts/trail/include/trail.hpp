// Trail is an on-chain voting platform for the Telos Blockchain Network that offers
// a full suite of voting services for both users and developers.
//
// @author Craig Branscom
// @contract trail
// @copyright see LICENSE.txt

//TODO: light ballots
//TODO: move units and fees into singleton
//TODO: add ignore<name> job_id to actions
//TODO: add standard asset param checks where applicable
//TODO: add bool for notify on transfer() and mint()

//TODO?: add inline for rebalance() after regvoter()
//TODO?: post job for wallet cleanup after destroy()
//TODO?: add secondary index on ballot expiration
//TODO?: make registry settings a map<name, bool>
//TODO?: trigger rebalance on transfers()
//TODO?: add delegation features

#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>
#include <string>
#include <algorithm>

using namespace eosio;
using namespace std;

CONTRACT trail : public contract {

public:

    //definitions
    const symbol TLOS_SYM = symbol("TLOS", 4);
    const symbol VOTE_SYM = symbol("VOTE", 4);
    const symbol TRAIL_SYM = symbol("TRAIL", 0);

    //units
    const uint32_t MIN_BALLOT_LENGTH = 86400; //1 day
    const uint32_t MIN_CLOSE_LENGTH = 259200; //3 days
    const uint16_t MAX_VOTE_RECEIPTS = 51;
    const uint32_t BALLOT_COOLDOWN = 259200;  //3 days in seconds

    //fees
    const asset BALLOT_LISTING_FEE = asset(150000, TLOS_SYM);
    const asset REGISTRY_LISTING_FEE = asset(250000, TLOS_SYM);
    const asset ARCHIVAL_BASE_FEE = asset(50000, TLOS_SYM);

    //job completion times
    const uint32_t FAST_COMPLETE = 86400; //1 day in seconds
    const uint32_t MEDIUM_COMPLETE = 604800; //1 week in seconds
    const uint32_t SLOW_COMPLETE = 2592000; //1 month in seconds (or longer)

    //token settings: transferable, burnable, reclaimable, modifiable

    //ballot statuses: setup, voting, closed, cancelled, archived

    //======================== registry actions ========================

    //create a new token registry
    ACTION newregistry(name publisher, asset max_supply);

    //toggle a registry setting
    ACTION toggle(symbol registry_sym, name setting_name);

    //mint new tokens to the recipient
    ACTION mint(name to, asset quantity, string memo);

    //transfer tokens
    ACTION transfer(name from, name to, asset quantity, string memo);

    //burn tokens from publisher balance
    ACTION burn(asset quantity);

    //reclaim tokens from voter
    ACTION reclaim(name voter, asset quantity);

    //adjust max supply
    ACTION modifymax(asset new_max_supply);

    //set new unlock auth
    ACTION setunlocker(symbol registry_sym, name new_unlock_acct, new_unlock_auth);

    //lock a token registry
    ACTION lockreg(symbol registry_sym);

    //unlock a token registry
    ACTION unlockreg(symbol registry_sym);

    //======================== ballot actions ========================

    //creates a new ballot
    ACTION newballot(name ballot_name, name category, name publisher, symbol token_sym, vector<name> initial_options);

    //edits ballots details
    ACTION editdetails(name ballot_name, string title, string description, string ballot_info);

    //sets ballots settings
    ACTION editsettings(name ballot_name, bool light_ballot, uint8_t max_options, bool allow_revoting);

    //adds an option to a ballot
    ACTION addoption(name ballot_name, name option_name);

    //removes an option from a ballot
    ACTION rmvoption(name ballot_name, name option_name);

    //readies a ballot for voting
    ACTION readyballot(name ballot_name, time_point_sec begin_time, time_point_sec end_time);

    //cancels a ballot
    ACTION cancelballot(name ballot_name, string memo);

    //closes a ballot after voting has concluded
    ACTION closeballot(name ballot_name);

    //deletes an expired ballot
    ACTION deleteballot(name ballot_name);

    //archives a ballot for a fee
    // ACTION archive(name ballot_name, time_point_sec archived_until);

    //======================== voter actions ========================

    //registers a new voter
    ACTION regvoter(name voter, symbol token_sym);

    //unregisters an existing voter
    ACTION unregvoter(name voter, symbol token_sym);

    //casts a vote on a ballot
    ACTION vote(name voter, name ballot_name, name option_name);

    //retracts a vote from a ballot
    ACTION unvote(name voter, name ballot_name, name option_name);

    //rebalance active votes after an unstake
    ACTION rebalance(name voter, symbol token_sym);

    //cleans the given number of old vote receipts
    ACTION cleanupvotes(name voter, uint16_t count, symbol token_sym);

    //attempts to clean all old vote receipts
    ACTION cleanhouse(name voter, symbol token_sym);

    //========================  actions ========================

    //
    

    //========== functions ==========

    //adds amount to account balance
    void add_balance(name owner, asset quantity);

    //subtracts amount from account balance
    void sub_balance(name owner, asset quantity);

    //returns true if registry exists
    bool is_registry(symbol token_sym);

    //returns true if the voter has a balance of the token symbol (even if its just 0)
    bool has_balance(name voter, symbol token_sym);

    
    
    //returns total stake from CPU + NET
    asset get_staked_tlos(name owner);

    //checks if a rebalance is needed and returns true if applied to ballot
    bool applied_rebalance(name ballot_name, asset delta, vector<name> options_to_rebalance);



    //======================== tables ========================


    //scope: get_self().value
    //ram: 
    TABLE registry {
        asset supply; //current supply
        asset max_supply; //maximum supply
        uint32_t voters; //number of open token accounts

        bool locked = false; //locks all settings
        name unlock_acct; //account name to unlock
        name unlock_auth; //authorization name to unlock

        map<name, bool> settings; //setting_name -> on/off

        name publisher; //registry owner
        string registry_info; //description or external link

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
        EOSLIB_SERIALIZE(registry, (supply)(max_supply)(voters)
            (locked)(unlock_acct)(unlock_auth)(settings)
            (publisher)(registry_info))
    };
    typedef multi_index<name("registries"), registry> registries_table;


    //scope: get_self().value
    //ram:
    TABLE ballot {
        name ballot_name;
        name category;
        name publisher;
        name status;

        string title; //markdown
        string description; //markdown
        string ballot_info; //typically IPFS link to content

        map<name, asset> options; //option name -> total votes
        symbol token_sym;
        uint32_t voters; //unique voters who have voted on ballot

        bool light_ballot; //see light ballot explanation
        uint8_t max_options; //number of options one user is allowed to vote on
        bool allow_revoting; //allows revoting for an option

        time_point_sec begin_time;
        time_point_sec end_time;

        uint64_t primary_key() const { return ballot_name.value; }
        EOSLIB_SERIALIZE(ballot, (ballot_name)(category)(publisher)(status)
            (title)(description)(ballot_info)
            (options)(token_sym)(voters)
            (light_ballot)(max_options)(allow_revoting)
            (begin_time)(end_time))
    };
    typedef multi_index<name("ballots"), ballot> ballots_table;


    //scope: owner.value
    //ram: 
    TABLE vote {
        name ballot_name;
        map<name, asset> options;
        time_point_sec expiration;

        uint64_t primary_key() const { return ballot_name.value; }
        uint64_t by_exp() const { return static_cast<uint64_t>(expiration); }
        EOSLIB_SERIALIZE(vote, (ballot_name)(options)(expiration))
    };
    typedef multi_index<name("votes"), vote,
        indexed_by<name("byexp"), const_mem_fun<vote, uint64_t, &vote::by_exp>>>
    votes_table;


    //scope: owner.value
    //ram: 
    TABLE account {
        asset balance;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }
        EOSLIB_SERIALIZE(account, (balance))
    };
    typedef multi_index<name("accounts"), account> accounts_table;

};