/**
 * Trail is an EOSIO-based voting service that allows users to create ballots that
 * are voted on by a network of registered voters. Trail also offers custom token
 * features that let any user to create their own voting token and configure settings 
 * to match a wide variety of intended use cases. 
 * 
 * @author Craig Branscom
 * @contract trail
 * @copyright see LICENSE.txt
 */

#include <eosiolib/asset.hpp>
#include <eosiolib/eosio.hpp>
#include <eosiolib/singleton.hpp>
#include <eosiolib/dispatcher.hpp>
#include <string>
#include <algorithm>

#include <eosiolib/print.hpp>

using namespace eosio;
using namespace std;

class [[eosio::contract("trail")]] trail : public contract {
    
    public:

    trail(name self, name code, datastream<const char*> ds);

    ~trail();

    //system defs
    struct user_resources {
        name owner;
        asset net_weight;
        asset cpu_weight;
        int64_t ram_bytes = 0;

        uint64_t primary_key()const { return owner.value; }
        EOSLIB_SERIALIZE( user_resources, (owner)(net_weight)(cpu_weight)(ram_bytes) )
    };

    typedef eosio::multi_index<name("userres"), user_resources> user_resources_table;

    //definitions
    const symbol VOTE_SYM = symbol("VOTE", 4);
    const uint32_t MIN_BALLOT_LENGTH = 86400; //1 day
    const uint32_t MIN_CLOSE_LENGTH = 259200; //3 days
    const uint16_t MAX_VOTE_RECEIPTS = 51; //TODO: move to token registry?
    const uint32_t BALLOT_COOLDOWN = 259200; //3 days in seconds

    enum ballot_status : uint8_t {
        SETUP, //0
        OPEN, //1
        CLOSED, //2
        CANCELLED, //3
        ARCHIVED //4
    };

    struct option {
        name option_name;
        asset votes;
        string info; //TODO: remove? ballot info could explain each option
    };
    // EOSLIB_SERIALIZE(option, (option_name)(votes)(info))

    struct token_settings {
        bool burnable = false;
        bool proxyable = false;
        bool seizable = false;
        bool transferable = false;
    };
    // EOSLIB_SERIALIZE(token_settings, 
    //     (burnable)(proxyable)
    //     (seizable)(transferable))

    //======================== tables ========================

    //@scope get_self().value
    //@ram
    TABLE ballot {
        name ballot_name;
        name category;
        name publisher;

        string title;
        string description;
        string info_link; //typically IPFS link to content

        vector<option> options;
        uint32_t unique_voters;
        uint8_t max_votable_options; //number of options user is allowed to vote on
        symbol voting_symbol;

        uint32_t begin_time;
        uint32_t end_time;
        uint8_t status;

        uint64_t primary_key() const { return ballot_name.value; }
        EOSLIB_SERIALIZE(ballot, (ballot_name)(category)(publisher)
            (title)(description)(info_link)
            (options)(unique_voters)(max_votable_options)(voting_symbol)
            (begin_time)(end_time)(status))
    };

    typedef multi_index<name("ballots"), ballot> ballots;

    //@scope get_self().value
    //@ram 
    TABLE registry { //TODO: rename to treasury?
        asset supply;
        asset max_supply;
        name publisher;

        uint32_t total_voters;
        uint32_t total_proxies;
        token_settings settings;
        string info_link;

        //TODO: maybe track max_vote_receipts here? or maybe in token_settings?

        uint64_t primary_key() const { return supply.symbol.code().raw(); }
        EOSLIB_SERIALIZE(registry, (supply)(max_supply)(publisher)
            (total_voters)(total_proxies)(settings)(info_link))
    };

    typedef multi_index<name("registries"), registry> registries;

    //@scope name.value
    //@ram 
    TABLE vote {
        name ballot_name;
        vector<name> option_names;
        asset amount;
        uint32_t expiration; //TODO: keep? could pull from ballot end time, removes ability to index by exp though

        uint64_t primary_key() const { return ballot_name.value; }
        uint64_t by_exp() const { return static_cast<uint64_t>(expiration); }
        EOSLIB_SERIALIZE(vote, (ballot_name)(option_names)(amount)(expiration))
    };

    typedef multi_index<name("votes"), vote,
        indexed_by<name("byexp"), const_mem_fun<vote, uint64_t, &vote::by_exp>>> votes;

    //@scope name.value
    //@ram 
    TABLE account {
        asset balance;
        uint16_t num_votes;

        uint64_t primary_key() const { return balance.symbol.code().raw(); }
        EOSLIB_SERIALIZE(account, (balance)(num_votes))
    };

    typedef multi_index<name("accounts"), account> accounts;

    //======================== ballot actions ========================

    //ballot actions
    ACTION newballot(name ballot_name, name category, name publisher, 
        string title, string description, string info_url, uint8_t max_votable_options,
        symbol voting_sym);

    //TODO: add max_votable_options to params
    ACTION upsertinfo(name ballot_name, name publisher, string title, string description, 
        string info_url, uint8_t max_votable_options);

    ACTION addoption(name ballot_name, name publisher,
        name option_name, string option_info);

    // ACTION rmvoption(name ballot_name, name publisher, name option_name);

    ACTION readyballot(name ballot_name, name publisher, uint32_t end_time);

    ACTION cancelballot(name ballot_name, name publisher);

    ACTION closeballot(name ballot_name, name publisher, uint8_t new_status);

    ACTION deleteballot(name ballot_name, name publisher);

    ACTION archive(name ballot_name, name publisher); //TODO: change to reqarchive()? require a TLOS transfer to archive?

    //======================== token actions ========================

    ACTION newtoken(name publisher, asset max_supply, token_settings settings, string info_url);

    ACTION mint(name publisher, name recipient, asset amount_to_mint);

    ACTION burn(name publisher, asset amount_to_burn);

    ACTION send(name sender, name recipient, asset amount, string memo);

    ACTION seize(name publisher, name owner, asset amount_to_seize);

    ACTION changemax(name publisher, asset max_supply_delta);

    // ACTION destroytoken();

    //======================== voter actions ========================

    //TODO: add inline for rebalance() ?
    ACTION regvoter(name owner, symbol token_sym);

    ACTION castvote(name voter, name ballot_name, name option);

    ACTION unvote(name voter, name ballot_name, name option);

    ACTION rebalance(name voter); //TODO: after unstake, require rebalancing before voting again?

    ACTION cleanupvotes(name voter, uint16_t count, symbol voting_sym);

    ACTION cleanhouse(name voter);

    ACTION unregvoter(name owner, symbol token_sym);

    //========== functions ==========

    //returns true if the given option_name is in the list of options
    bool is_option_in_ballot(name option_name, vector<option> options);

    //returns true if the given option_name is in the list of names
    bool is_option_in_receipt(name option_name, vector<name> options_voted);

    //returns the index of the vector that points to the option_name
    int get_option_index(name option_name, vector<option> options);

    //returns true if the voter has a balance of the token symbol (even if its just 0)
    bool has_token_balance(name voter, symbol sym);

    //returns total stake from CPU + NET
    asset get_staked_tlos(name owner);

    //checks if a rebalance is needed and returns true if applied to ballot
    bool applied_rebalance(name ballot_name, asset delta, vector<name> options_to_rebalance);

    //adds amount to account balance
    void add_balance(name owner, asset amount);

    //subtracts amount from account balance
    void sub_balance(name owner, asset amount);

};