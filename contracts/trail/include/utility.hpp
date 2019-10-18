// Utility file with functions and definitions for interacting with external contracts.
// 
// @author Craig Branscom

#pragma once

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio.system/eosio.system.hpp>

using namespace std;
using namespace eosio;

//TODO: add get_staked_rex()

using user_resources = eosiosystem::user_resources;
using user_resources_table = eosiosystem::user_resources_table;

//defined in 
asset get_staked_tlos(name owner) {
    user_resources_table userres(name("eosio"), owner.value);
    auto r = userres.find(owner.value);

    int64_t amount = 0;

    if (r != userres.end()) {
        auto res = *r;
        amount = (res.cpu_weight.amount + res.net_weight.amount);
    }
    
    return asset(amount, symbol("TLOS", 4));
}
