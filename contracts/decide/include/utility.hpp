// Utility file with functions and definitions for interacting with external contracts.
// 
// @author Craig Branscom

#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio.system/eosio.system.hpp>

using namespace std;
using namespace eosio;

using user_resources = eosiosystem::user_resources;
using del_bandwidth_table = eosiosystem::del_bandwidth_table;
using rex_bal_table = eosiosystem::rex_balance_table;

//eosio.token account table
struct eosio_account {
    asset balance;

    uint64_t primary_key() const { return balance.symbol.code().raw(); }

    EOSLIB_SERIALIZE(eosio_account, (balance))
};
typedef multi_index<name("accounts"), eosio_account> eosio_accounts_table;

//defined in 
asset get_staked_tlos(name owner) {
    del_bandwidth_table delband(name("eosio"), owner.value);
    auto r = delband.find(owner.value);

    int64_t amount = 0;

    if (r != delband.end()) {
        amount = (r->cpu_weight.amount + r->net_weight.amount);
    }
    
    return asset(amount, symbol("TLOS", 4));
}

asset get_tlos_in_rex(name owner) {
    rex_bal_table rexbals(name("eosio"), name("eosio").value);
    auto rb = rexbals.find(owner.value);

    int64_t amount = 0;

    if (rb != rexbals.end()) {
        amount = rb->vote_stake.amount;
    }

    return asset(amount, symbol("TLOS", 4));
}
