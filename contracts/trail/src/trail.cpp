#include <trail.hpp>
#include <utility.hpp>

#include <eosio.token/eosio.token.hpp>

using namespace trailservice;

trail::trail(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

trail::~trail() {}

//======================== admin actions ========================

ACTION trail::setconfig(string trail_version, bool set_defaults) {
    
    //authenticate
    require_auth(get_self());

    //initialize
    map<name, asset> new_fees;
    map<name, uint32_t> new_times;

    if (set_defaults) {
        //set default fees
        new_fees[name("ballot")] = asset(300000, TLOS_SYM); //30 TLOS
        new_fees[name("treasury")] = asset(10000000, TLOS_SYM); //1000 TLOS
        new_fees[name("archival")] = asset(30000, TLOS_SYM); //3 TLOS (per day)
        new_fees[name("committee")] = asset(100000, TLOS_SYM); //100 TLOS

        //set default times
        new_times[name("minballength")] = uint32_t(86400); //1 day in seconds
        new_times[name("balcooldown")] = uint32_t(432000); //5 days in seconds
        new_times[name("forfeittime")] = uint32_t(864000); //10 days in seconds
    }

    //open config singleton
    config_singleton configs(get_self(), get_self().value);

    //build new configs
    config new_config = {
        trail_version, //trail_version
        asset(0, TLOS_SYM),
        new_fees, //fees
        new_times //times
    };

    //set new config
    configs.set(new_config, get_self());
}

ACTION trail::updatefee(name fee_name, asset fee_amount) {
    
    //authenticate
    require_auth(get_self());

    //open configs singleton
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();
    
    //validate
    check(fee_amount.symbol == TLOS_SYM,  "fee symbol must be TLOS");
    check(fee_amount >= asset(0, TLOS_SYM), "fee amount must be a positive number");

    //initialize
    config new_conf = conf;

    //update fee
    new_conf.fees[fee_name] = fee_amount;

    //update fee
    configs.set(new_conf, get_self());

}

ACTION trail::updatetime(name time_name, uint32_t length) {
    
    //authenticate
    require_auth(get_self());

    //open configs singleton
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //validate
    check(length >= 1, "length must be a positive number");

    //initialize
    config new_conf = conf;

    //update time name with new length
    new_conf.times[time_name] = length;

    //update time
    configs.set(new_conf, get_self());

}

ACTION trail::withdraw(name voter, asset quantity) {
    
    //authenticate
    require_auth(voter);
    
    //open accounts table, get account
    accounts_table tlos_accounts(get_self(), voter.value);
    auto& acct = tlos_accounts.get(TLOS_SYM.code().raw(), "account not found");

    //validate
    check(quantity.symbol == TLOS_SYM, "can only withdraw TLOS");
    check(acct.balance >= quantity, "insufficient balance");
    check(quantity > asset(0, TLOS_SYM), "must withdraw a positive amount");

    //update balances
    tlos_accounts.modify(acct, same_payer, [&](auto& col) {
        col.balance -= quantity;
    });

    config_singleton configs(get_self(), get_self().value);
    auto config = configs.get();

    config.total_deposits -= quantity;

    configs.set(config, get_self());

    //transfer to eosio.token
    //inline trx requires trailservice@active to have trailservice@eosio.code
    //TODO: replace with action handler
    token::transfer_action transfer_act("eosio.token"_n, { get_self(), active_permission });
    transfer_act.send(get_self(), voter, quantity, std::string("trailservice withdrawal"));
}

//========== notification methods ==========

void trail::catch_delegatebw(name from, name receiver, asset stake_net_quantity, asset stake_cpu_quantity, bool transfer) {
    //sync external stake with internal stake
    sync_external_account(from, VOTE_SYM, stake_net_quantity.symbol);
}

void trail::catch_undelegatebw(name from, name receiver, asset unstake_net_quantity, asset unstake_cpu_quantity) {
    //sync external stake with internal stake
    sync_external_account(from, VOTE_SYM, unstake_net_quantity.symbol);
}

void trail::catch_transfer(name from, name to, asset quantity, string memo) {
    //get initial receiver contract
    name rec = get_first_receiver();

    //validate
    if (rec == token_account && from != get_self() && quantity.symbol == TLOS_SYM) {
        
        //parse memo
        //NOTE: keyword should maybe be "refund" or "return"
        if (memo == std::string("skip")) //skips emplacement if memo is skip
            return;
        
        //open accounts table, search for account
        accounts_table accounts(get_self(), from.value);
        auto acct = accounts.find(TLOS_SYM.code().raw());

        //emplace account if not found, update if exists
        if (acct == accounts.end()) { //no account
            accounts.emplace(get_self(), [&](auto& col) {
                col.balance = quantity;
            });
        } else { //exists
            accounts.modify(*acct, same_payer, [&](auto& col) {
                col.balance += quantity;
            });
        }

        config_singleton configs(get_self(), get_self().value);
        auto config = configs.get();

        config.total_deposits += quantity;

        configs.set(config, get_self());
    } else if (rec == token_account && from == get_self() && quantity.symbol == TLOS_SYM) {
        config_singleton configs(get_self(), get_self().value);
        auto config = configs.get();

        asset total_transferable = (token::get_balance("eosio.token"_n, get_self(), TLOS_SYM.code()) + quantity) - config.total_deposits;
        
        check(total_transferable >= quantity, "trailservice lacks the liquid TLOS to make this transfer");
    }
}

//========== utility methods ==========

void trail::add_liquid(name voter, asset quantity) {
    //open voters table, get voter
    voters_table to_voters(get_self(), voter.value);
    auto& to_voter = to_voters.get(quantity.symbol.code().raw(), "add_liquid: voter not found");

    //add quantity to liquid
    to_voters.modify(to_voter, same_payer, [&](auto& col) {
        col.liquid += quantity;
    });
}

void trail::sub_liquid(name voter, asset quantity) {
    //open voters table, get voter
    voters_table from_voters(get_self(), voter.value);
    auto& from_voter = from_voters.get(quantity.symbol.code().raw(), "sub_liquid: voter not found");

    //validate
    check(from_voter.liquid >= quantity, "insufficient liquid amount");

    //subtract quantity from liquid
    from_voters.modify(from_voter, same_payer, [&](auto& col) {
        col.liquid -= quantity;
    });
}

void trail::add_stake(name voter, asset quantity) {
    //open voters table, get voter
    voters_table to_voters(get_self(), voter.value);
    auto& to_voter = to_voters.get(quantity.symbol.code().raw(), "add_stake: voter not found");

    //add quantity to stake
    to_voters.modify(to_voter, same_payer, [&](auto& col) {
        col.staked += quantity;
        col.staked_time = time_point_sec(current_time_point());
    });
}

void trail::sub_stake(name voter, asset quantity) {
    //open voters table, get voter
    voters_table from_voters(get_self(), voter.value);
    auto& from_voter = from_voters.get(quantity.symbol.code().raw(), "sub_stake: voter not found");

    //validate
    check(from_voter.staked >= quantity, "insufficient staked amount");

    //subtract quantity from stake
    from_voters.modify(from_voter, same_payer, [&](auto& col) {
        col.staked -= quantity;
        col.staked_time = time_point_sec(current_time_point());
    });
}

bool trail::valid_category(name category) {
    switch (category.value) {
        case (name("proposal").value):
            return true;
        case (name("referendum").value):
            return true;
        case (name("election").value):
            return true;
        case (name("poll").value):
            return true;
        case (name("leaderboard").value):
            return true;
        default:
            return false;
    }
}

bool trail::valid_voting_method(name voting_method) {
    switch (voting_method.value) {
        case (name("1acct1vote").value):
            return true;
        case (name("1tokennvote").value):
            return true;
        case (name("1token1vote").value):
            return true;
        case (name("1tsquare1v").value):
            return true;
        case (name("quadratic").value):
            return true;
        case (name("ranked").value):
            check(false, "ranked voting method feature under development");
            return true;
        default:
            return false;
    }
}

bool trail::valid_access_method(name access_method) {
    switch (access_method.value) {
        case (name("public").value):
            return true;
        case (name("private").value):
            return true;
        case (name("invite").value):
            return true;
        case (name("membership").value):
            check(false, "membership access feature under development");
            return true;
        default:
            return false;
    }
}

void trail::require_fee(name account_name, asset fee) {
    //open accounts table, get TLOS balance
    accounts_table tlos_accounts(get_self(), account_name.value);
    auto& tlos_acct = tlos_accounts.get(TLOS_SYM.code().raw(), "TLOS balance not found");

    //validate
    check(tlos_acct.balance >= fee, "insufficient funds to cover fee");

    config_singleton configs(get_self(), get_self().value);
    auto config = configs.get();

    config.total_deposits -= fee;

    configs.set(config, get_self());

    //charge fee
    tlos_accounts.modify(tlos_acct, same_payer, [&](auto& col) {
        col.balance -= fee;
    });
}

void trail::sync_external_account(name voter, symbol internal_symbol, symbol external_symbol) {
    
    //initialize
    asset tlos_stake;

    //check external symbol is TLOS, if not then return
    if (external_symbol == TLOS_SYM) {
        tlos_stake = get_staked_tlos(voter);
        tlos_stake += get_tlos_in_rex(voter);
        check(internal_symbol == VOTE_SYM, "internal symbol must be VOTE for external TLOS");
    } else {
        check(false, "syncing external accounts is under development");
        return;
    }

    //open voters table, search for voter
    voters_table voters(get_self(), voter.value);
    auto vtr_itr = voters.find(internal_symbol.code().raw());

    //subtract from VOTE stake
    if (vtr_itr != voters.end()) { //voter exists

        //open treasuries table, search for treasury
        treasuries_table treasuries(get_self(), get_self().value);
        auto trs_itr = treasuries.find(internal_symbol.code().raw());

        //check if treasury exists (should always be true)
        if (trs_itr == treasuries.end())
            return;
            
        //calc delta
        asset delta = asset(tlos_stake.amount - vtr_itr->staked.amount, internal_symbol);

        //apply delta to supply
        treasuries.modify(*trs_itr, same_payer, [&](auto& col) {
            col.supply += asset(delta.amount, internal_symbol);
        });

        //mirror tlos_stake to internal_symbol stake
        voters.modify(*vtr_itr, same_payer, [&](auto& col) {
            col.staked = asset(tlos_stake.amount, internal_symbol);
        });

    }
}
