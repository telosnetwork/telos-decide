// Telos Decide is an on-chain governance engine for the Telos Blockchain Network that provides
// a full suite of voting services for users and developers.
//
// @author Craig Branscom
// @contract decide
// @version v2.0.0
// @copyright see LICENSE.txt

#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/singleton.hpp>

// #include <eosio.token/eosio.token.hpp>

#include <cmath>

using namespace eosio;
using namespace std;

namespace decidespace {
    
    CONTRACT decide : public contract {

    public:

        decide(name self, name code, datastream<const char*> ds);

        ~decide();

        //reserved symbols
        static constexpr symbol TLOS_SYM = symbol("TLOS", 4);
        static constexpr symbol VOTE_SYM = symbol("VOTE", 4);

        //reserved permission names
        static constexpr name active_permission = "active"_n;

        //reserved account names
        static constexpr name token_account = "eosio.token"_n;

        //reserved names
        static constexpr name ballot_n = "ballot"_n;
        static constexpr name worker_n = "worker"_n;
        static constexpr name treasury_n = "treasury"_n;
        static constexpr name archival_n = "archival"_n;
        static constexpr name committee_n = "committee"_n;
        static constexpr name proposal_n = "proposal"_n;
        static constexpr name referendum_n = "referendum"_n;
        static constexpr name election_n = "election"_n;
        static constexpr name poll_n = "poll"_n;
        static constexpr name leaderboard_n = "leaderboard"_n;
        
        //treasury settings: transferable, burnable, reclaimable, stakeable, unstakeable, maxmutable

        //treasury access: public, private, invite

        //ballot statuses: setup, voting, closed, cancelled, archived

        //ballot settings: lightballot, revotable, voteliquid, votestake

        //voting methods: 1acct1vote, 1tokennvote, 1token1vote, 1tsquare1v, quadratic

        //ballot categories: proposal, referendum, election, poll, leaderboard

        //======================== admin actions ========================

        //initialize contract
        ACTION init(string app_version);

        //set new app version
        ACTION setversion(string new_app_version);

        //updates fee amount
        ACTION updatefee(name fee_name, asset fee_amount);

        //updates time length
        ACTION updatetime(name time_name, uint32_t length);

        //======================== treasury actions ========================

        //create a new treasury
        ACTION newtreasury(name manager, asset max_supply, name access);
        using newtreasury_action = action_wrapper<"newtreasury"_n, &decide::newtreasury>;

        //edit treasury title, description, and icon
        ACTION edittrsinfo(symbol treasury_symbol, string title, string description, string icon);
        using edittrsinfo_action = action_wrapper<"edittrsinfo"_n, &decide::edittrsinfo>;

        //toggle a treasury setting
        ACTION toggle(symbol treasury_symbol, name setting_name);
        using toggle_action = action_wrapper<"toggle"_n, &decide::toggle>;

        //mint new tokens to the recipient
        ACTION mint(name to, asset quantity, string memo);
        using mint_action = action_wrapper<"mint"_n, &decide::mint>;

        //transfer tokens
        ACTION transfer(name from, name to, asset quantity, string memo);
        using transfer_action = action_wrapper<"transfer"_n, &decide::transfer>;

        //burn tokens from manager balance
        ACTION burn(asset quantity, string memo);
        using burn_action = action_wrapper<"burn"_n, &decide::burn>;

        //reclaim tokens from voter to the manager balance
        ACTION reclaim(name voter, asset quantity, string memo);
        using reclaim_action = action_wrapper<"reclaim"_n, &decide::reclaim>;

        //change max supply
        ACTION mutatemax(asset new_max_supply, string memo);
        using mutatemax_action = action_wrapper<"mutatemax"_n, &decide::mutatemax>;

        //set new unlock auth
        ACTION setunlocker(symbol treasury_symbol, name new_unlock_acct, name new_unlock_auth);
        using setunlocker_action = action_wrapper<"setunlocker"_n, &decide::setunlocker>;

        //lock a treasury
        ACTION lock(symbol treasury_symbol);
        using lock_action = action_wrapper<"lock"_n, &decide::lock>;

        //unlock a treasury
        ACTION unlock(symbol treasury_symbol);
        using unlock_action = action_wrapper<"unlock"_n, &decide::unlock>;

        //======================== payroll actions ========================

        //adds tokens to specified payroll
        ACTION addfunds(name from, symbol treasury_symbol, asset quantity);

        //edit pay rate
        ACTION editpayrate(symbol treasury_symbol, uint32_t period_length, asset per_period);

        //======================== ballot actions ========================

        //creates a new ballot
        ACTION newballot(name ballot_name, name category, name publisher,  
            symbol treasury_symbol, name voting_method, vector<name> initial_options);
        using newballot_action = action_wrapper<"newballot"_n, &decide::newballot>;

        //edits ballots details
        ACTION editdetails(name ballot_name, string title, string description, string content);
        using editdetails_action = action_wrapper<"editdetails"_n, &decide::editdetails>;

        //toggles ballot settings
        ACTION togglebal(name ballot_name, name setting_name);
        using togglebal_action = action_wrapper<"togglebal"_n, &decide::togglebal>;

        //edits ballot min and max options
        ACTION editminmax(name ballot_name, uint8_t new_min_options, uint8_t new_max_options);
        using editminmax_action = action_wrapper<"editminmax"_n, &decide::editminmax>;

        //adds an option to a ballot
        ACTION addoption(name ballot_name, name new_option_name);
        using addoption_action = action_wrapper<"addoption"_n, &decide::addoption>;

        //removes an option from a ballot
        ACTION rmvoption(name ballot_name, name option_name);
        using rmvoption_action = action_wrapper<"rmvoption"_n, &decide::rmvoption>;

        //opens a ballot for voting
        ACTION openvoting(name ballot_name, time_point_sec end_time);
        using openvoting_action = action_wrapper<"openvoting"_n, &decide::openvoting>;

        //cancels a ballot
        ACTION cancelballot(name ballot_name, string memo);
        using cancelballot_action = action_wrapper<"cancelballot"_n, &decide::cancelballot>;

        //deletes an expired ballot
        ACTION deleteballot(name ballot_name);
        using deleteballot_action = action_wrapper<"deleteballot"_n, &decide::deleteballot>;

        //posts results from a light ballot before closing
        ACTION postresults(name ballot_name, map<name, asset> light_results, uint32_t total_voters);
        using postresults_action = action_wrapper<"postresults"_n, &decide::postresults>;

        //closes voting on a ballot and post final results
        ACTION closevoting(name ballot_name, bool broadcast);
        using closevoting_action = action_wrapper<"closevoting"_n, &decide::closevoting>;

        //broadcast ballot results
        ACTION broadcast(name ballot_name, map<name, asset> final_results, uint32_t total_voters);
        using broadcast_action = action_wrapper<"broadcast"_n, &decide::broadcast>;

        //archives a ballot for a fee
        ACTION archive(name ballot_name, time_point_sec archived_until);
        using archive_action = action_wrapper<"archive"_n, &decide::archive>;

        //unarchives a ballot after archival time has expired
        ACTION unarchive(name ballot_name, bool force);
        using unarchive_action = action_wrapper<"unarchive"_n, &decide::unarchive>;

        //======================== voter actions ========================

        //registers a new voter
        ACTION regvoter(name voter, symbol treasury_symbol, optional<name> referrer);

        //unregisters an existing voter
        ACTION unregvoter(name voter, symbol treasury_symbol);

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

        //refreshes external balance
        ACTION refresh(name voter);

        //======================== worker actions ========================

        //rebalance an unbalanced vote
        ACTION rebalance(name voter, name ballot_name, optional<name> worker);

        //cleans up an expired vote
        ACTION cleanupvote(name voter, name ballot_name, optional<name> worker);

        //unregisters an existing worker
        ACTION forfeitwork(name worker_name, symbol treasury_symbol);

        //pays a worker
        ACTION claimpayment(name worker_name, symbol treasury_symbol);

        //withdraws TLOS balance to eosio.token
        ACTION withdraw(name voter, asset quantity);

        //======================== committee actions ========================

        //registers a new committee for a treasury
        ACTION regcommittee(name committee_name, string committee_title,
            symbol treasury_symbol, vector<name> initial_seats, name registree);
        using regcommittee_action = action_wrapper<"regcommittee"_n,&decide::regcommittee>;

        //adds a committee seat
        ACTION addseat(name committee_name, symbol treasury_symbol, name new_seat_name);
        using addseat_action = action_wrapper<"addseat"_n, &decide::addseat>;

        //removes a committee seat
        ACTION removeseat(name committee_name, symbol treasury_symbol, name seat_name);
        using removeseat_action = action_wrapper<"removeseat"_n, &decide::removeseat>;

        //assigns a new member to a committee seat
        ACTION assignseat(name committee_name, symbol treasury_symbol, name seat_name, name seat_holder, string memo);
        using assignseat_action = action_wrapper<"assignseat"_n, &decide::assignseat>;

        //sets updater account and auth
        ACTION setupdater(name committee_name, symbol treasury_symbol, name updater_account, name updater_auth);
        using setupdater_action = action_wrapper<"setupdater"_n, &decide::setupdater>;

        //deletes a committee
        ACTION delcommittee(name committee_name, symbol treasury_symbol, string memo);
        using delcommittee_action = action_wrapper<"delcommitee"_n, &decide::delcommittee>;

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

        //charges a fee to a TLOS or TLOSD balance
        void require_fee(name account_name, asset fee);

        //logs rebalance work
        void log_rebalance_work(name worker, symbol treasury_symbol, asset volume, uint16_t count);

        //logs cleanup work
        void log_cleanup_work(name worker, symbol treasury_symbol, uint16_t count);

        //syncs an exernal account balance with a linked voter balance
        void sync_external_account(name voter, symbol internal_symbol, symbol external_symbol);

        //calculates vote mapping
        map<name, asset> calc_vote_weights(symbol treasury_symbol, name voting_method, 
        vector<name> selections,  asset raw_vote_weight);

        //======================== tables ========================

        //scope: singleton
        //ram: 
        TABLE config {
            string app_name;
            string app_version;
            asset total_deposits;
            map<name, asset> fees; //ballot, treasury, archival
            map<name, uint32_t> times; //balcooldown, minballength, forfeittime

            EOSLIB_SERIALIZE(config, (app_name)(app_version)(total_deposits)(fees)(times))
        };
        typedef singleton<name("config"), config> config_singleton;

        //scope: get_self().value
        //ram: 
        TABLE treasury {
            asset supply; //current supply
            asset max_supply; //maximum supply
            name access; //public, private, invite, membership
            name manager; //treasury manager

            string title;
            string description;
            string icon;

            uint32_t voters;
            uint32_t delegates;
            uint32_t committees;
            uint32_t open_ballots;

            bool locked; //locks all settings
            name unlock_acct; //account name to unlock
            name unlock_auth; //authorization name to unlock
            map<name, bool> settings; //setting_name -> on/off

            uint64_t primary_key() const { return supply.symbol.code().raw(); }
            EOSLIB_SERIALIZE(treasury, 
                (supply)(max_supply)(access)(manager)
                (title)(description)(icon)
                (voters)(delegates)(committees)(open_ballots)
                (locked)(unlock_acct)(unlock_auth)(settings))
        };
        typedef multi_index<name("treasuries"), treasury> treasuries_table;

        //scope: treasury_symbol.code().raw()
        //ram:
        TABLE payroll {
            name payroll_name; //workers, delegates
            asset payroll_funds; //TLOS, TLOSD

            uint32_t period_length; //in seconds
            asset per_period; //amount made claimable from payroll funds every period
            time_point_sec last_claim_time; //last time pay was claimed

            asset claimable_pay; //funds tapped when claimpayment() is called
            name payee; //craig.tf, workers, delegates

            uint64_t primary_key() const { return payroll_name.value; }
            EOSLIB_SERIALIZE(payroll, (payroll_name)(payroll_funds)
                (period_length)(per_period)(last_claim_time)
                (claimable_pay)(payee))
        };
        typedef multi_index<name("payrolls"), payroll> payrolls_table;

        //scope: treasury_symbol.code().raw()
        //ram: 
        TABLE labor_bucket {
            name payroll_name; //workers, delegates
            map<name, asset> claimable_volume; //rebalvolume, dgatevolume
            map<name, uint32_t> claimable_events; //rebalcount, cleancount, cleanspeed, rebalspeed, dgatecount

            uint64_t primary_key() const { return payroll_name.value; }
            EOSLIB_SERIALIZE(labor_bucket, (payroll_name)(claimable_volume)(claimable_events))
        };
        typedef multi_index<name("laborbuckets"), labor_bucket> laborbuckets_table;

        //scope: treasury_symbol.code().raw()
        //ram:
        TABLE labor {
            name worker_name;
            time_point_sec start_time; //time point work was first credited
            map<name, asset> unclaimed_volume; //rebalvolume
            map<name, uint32_t> unclaimed_events; //rebalcount, cleancount, cleanspeed, rebalspeed

            uint64_t primary_key() const { return worker_name.value; }
            EOSLIB_SERIALIZE(labor, (worker_name)(start_time)(unclaimed_volume)(unclaimed_events))
        };
        typedef multi_index<name("labors"), labor> labors_table;

        //scope: get_self().value
        //ram:
        TABLE ballot {
            name ballot_name;
            name category; //proposal, referendum, election, poll, leaderboard
            name publisher;
            name status; //setup, voting, closed, cancelled, archived

            string title; //markdown
            string description; //markdown
            string content; //IPFS link to content or markdown

            symbol treasury_symbol; //treasury used for counting votes
            name voting_method; //1acct1vote, 1tokennvote, 1token1vote, 1tsquare1v, quadratic
            uint8_t min_options; //minimum options per voter
            uint8_t max_options; //maximum options per voter
            map<name, asset> options; //option name -> total weighted votes

            uint32_t total_voters; //number of voters who voted on ballot
            uint32_t total_delegates; //number of delegates who voted on ballot
            asset total_raw_weight; //total raw weight cast on ballot
            uint32_t cleaned_count; //number of expired vote receipts cleaned
            map<name, bool> settings; //setting name -> on/off
            
            time_point_sec begin_time; //time that voting begins
            time_point_sec end_time; //time that voting closes

            uint64_t primary_key() const { return ballot_name.value; }
            uint64_t by_category() const { return category.value; }
            uint64_t by_status() const { return status.value; }
            uint64_t by_symbol() const { return treasury_symbol.code().raw(); }
            uint64_t by_end_time() const { return static_cast<uint64_t>(end_time.utc_seconds); }
            
            EOSLIB_SERIALIZE(ballot, 
                (ballot_name)(category)(publisher)(status)
                (title)(description)(content)
                (treasury_symbol)(voting_method)(min_options)(max_options)(options)
                (total_voters)(total_delegates)(total_raw_weight)(cleaned_count)(settings)
                (begin_time)(end_time))
        };
        typedef multi_index<name("ballots"), ballot,
            indexed_by<name("bycategory"), const_mem_fun<ballot, uint64_t, &ballot::by_category>>,
            indexed_by<name("bystatus"), const_mem_fun<ballot, uint64_t, &ballot::by_status>>,
            indexed_by<name("bysymbol"), const_mem_fun<ballot, uint64_t, &ballot::by_symbol>>,
            indexed_by<name("byendtime"), const_mem_fun<ballot, uint64_t, &ballot::by_end_time>>
        > ballots_table;

        //scope: ballot_name.value
        //ram: 
        TABLE vote {
            name voter;
            bool is_delegate;
            asset raw_votes;
            map<name, asset> weighted_votes;
            time_point_sec vote_time;
            
            name worker;
            uint8_t rebalances;
            asset rebalance_volume;

            uint64_t primary_key() const { return voter.value; }
            uint64_t by_time() const { return static_cast<uint64_t>(vote_time.utc_seconds); }
            EOSLIB_SERIALIZE(vote, 
                (voter)(is_delegate)(raw_votes)(weighted_votes)(vote_time)
                (worker)(rebalances)(rebalance_volume))
        };
        typedef multi_index<name("votes"), vote,
            indexed_by<name("bytime"), const_mem_fun<vote, uint64_t, &vote::by_time>>
        > votes_table;

        //scope: voter.value
        //ram: 
        TABLE voter {
            asset liquid;

            asset staked;
            time_point_sec staked_time;

            asset delegated;
            name delegated_to;
            time_point_sec delegation_time;

            uint64_t primary_key() const { return liquid.symbol.code().raw(); }
            EOSLIB_SERIALIZE(voter,
                (liquid)
                (staked)(staked_time)
                (delegated)(delegated_to)(delegation_time))
        };
        typedef multi_index<name("voters"), voter> voters_table;

        //scope: treasury_symbol.code().raw()
        //ram: 
        TABLE delegate {
            name delegate_name;
            asset total_delegated;
            uint32_t constituents;

            uint64_t primary_key() const { return delegate_name.value; }
            EOSLIB_SERIALIZE(delegate, (delegate_name)(total_delegated)(constituents))
        };
        typedef multi_index<name("delegates"), delegate> delegates_table;

        //scope: treasury_symbol.code().raw()
        //ram: 
        TABLE committee {
            string committee_title;
            name committee_name;

            symbol treasury_symbol;
            map<name, name> seats; //seat_name -> seat_holder (0 if empty)

            name updater_acct; //account name that can update committee members
            name updater_auth; //auth name that can update committee members

            uint64_t primary_key() const { return committee_name.value; }
            EOSLIB_SERIALIZE(committee, 
                (committee_title)(committee_name)
                (treasury_symbol)(seats)
                (updater_acct)(updater_auth))
        };
        typedef multi_index<name("committees"), committee> committees_table;

        //scope: get_self().value
        //ram:
        TABLE archival {
            name ballot_name;
            time_point_sec archived_until;

            uint64_t primary_key() const { return ballot_name.value; }
            EOSLIB_SERIALIZE(archival, (ballot_name)(archived_until))
        };
        typedef multi_index<name("archivals"), archival> archivals_table;

        //scope: treasury_symbol.code().raw()
        //ram:
        TABLE featured_ballot {
            name ballot_name;
            time_point_sec featured_until;

            uint64_t primary_key() const { return ballot_name.value; }
            EOSLIB_SERIALIZE(featured_ballot, (ballot_name)(featured_until))
        };
        typedef multi_index<name("featured"), featured_ballot> featured_table;

        //scope: account_name.value
        //ram: 
        TABLE account {
            asset balance;

            uint64_t primary_key() const { return balance.symbol.code().raw(); }
            EOSLIB_SERIALIZE(account, (balance))
        };
        typedef multi_index<name("accounts"), account> accounts_table;

    };
}