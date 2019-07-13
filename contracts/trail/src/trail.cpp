#include "../include/trail.hpp"

trail::trail(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

trail::~trail() {}



//======================== registry actions ========================

ACTION trail::newregistry(name publisher, asset max_supply) {
    //authenticate
    require_auth(publisher);

    new_sym = max_supply.symbol;

    //open registries table
    registries_table registries(get_self(), get_self().value);
    auto reg = registries.find(new_sym.code().raw());

    //validate
    check(reg == registries.end(), "registry already exists");

    //check reserved symbols
    if (publisher != get_self()) {
        check(new_sym.code().raw() != TLOS_SYM.code().raw(), "TLOS symbol is reserved");
        check(new_sym.code().raw() != VOTE_SYM.code().raw(), "VOTE symbol is reserved");
        check(new_sym.code().raw() != TRAIL_SYM.code().raw(), "TRAIL symbol is reserved");
    }

    map<name, bool> initial_settings;
    initial_settings[name("transferable")] = false;
    initial_settings[name("burnable")] = false;
    initial_settings[name("reclaimable")] = false;
    initial_settings[name("modifiable")] = false;

    //emplace new registry
    registries.emplace(get_self(), [&](auto& col){
        col.supply = asset(0, new_sym);
        col.max_supply = max_supply;
        col.voters = uint32_t(0);
        col.locked = false;
        col.unlock_acct = publisher;
        col.unlock_auth = name("active");
        col.settings = initial_settings;
        col.publisher = publisher;
        col.registry_info = "";
    });

}

ACTION trail::toggle(symbol registry_sym, name setting_name) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(registry_sym.code().raw());

    //authenticate
    require_auth(reg.publisher);

    //validate
    check(!reg.locked, "registry is locked");
    auto itr = reg.settings.find(setting_name);
    check(itr != reg.end(), "setting not found");

    //update setting
    registries.modify(lic, same_payer, [&](auto& col) {
        col.settings[setting_name] = !settings[setting_name];
    });
}

ACTION trail::mint(name to, asset quantity, string memo) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(quantity.symbol.code().raw(), "registry with symbol not found");

    //authenticate
    require_auth(reg.publisher);

    //open accounts table, get account
    accounts_table accounts(get_self(), to.value);
    auto& acct = accounts.get(quantity.symbol.code().raw(), "account not found");

    //validate
    check(is_account(to), "to account doesn't exist");
    check(reg.supply + quantity <= reg.max_supply, "minting would breach max supply");
    check(quantity > asset(0, quantity.symbol), "must mint a positive quantity");
    check(quantity.is_valid(), "invalid quanitity");

    //update to balance
    accounts.modify(acct, same_payer, [&](auto& col) {
        col.balance += quantity;
    });

    //update registry supply
    registries.modify(reg, same_payer, [&](auto& col) {
        col.supply += quantity;
    });
}

ACTION trail::transfer(name from, name to, asset quantity, string memo) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(quantity.symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(from);

    //validate
    check(sender != recipient, "cannot send tokens to yourself");
    check(is_account(to), "to account doesn't exist");
    check(reg.settings.[name("transferable")], "token is not transferable");
    check(quantity.amount > 0, "must transfer positive quantity");
    check(quantity.is_valid(), "invalid quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //sub quantity from sender
    sub_balance(from, quantity);

    //add quantity to recipient
    add_balance(to, quantity);
}

ACTION trail::burn(asset quantity) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(quantity.symbol.code().raw(), "registry not found");

    //open accounts table, get publisher account
    accounts accounts(get_self(), reg.publisher.value);
    auto& pub_acct = accounts.get(quantity.symbol.code().raw(), "account not found");

    //authenticate
    require_auth(reg.publisher);

    //validate
    check(reg.settings[name("burnable")], "token is not burnable");
    check(reg.supply - quantity >= asset(0, quantity.symbol), "cannot burn more tokens than exist");
    check(pub_acct.balance >= quantity, "cannot burn more tokens than owned");
    check(quantity > asset(0, quantity.symbol), "must burn a positive quantity");
    check(quantity.is_valid(), "invalid quantity");

    //update publisher balance
    accounts.modify(pub_acct, same_payer, [&](auto& col) {
        col.balance -= quantity;
    });
}

ACTION trail::reclaim(name voter, asset quantity) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(quantity.symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.publisher);

    //validate
    check(reg.settings[name("reclaimable")], "token is not reclaimable");
    check(reg.publisher != voter, "cannot reclaim tokens from yourself");
    check(is_account(voter), "voter account doesn't exist on-chain");
    check(quantity.is_valid(), "invalid amount");
    check(quantity.amount > 0, "must reclaim positive amount");

    //sub quantity from voter
    sub_balance(voter, quantity);

    //add quantity to publisher
    add_balance(reg.publisher, quantity);
}

ACTION trail::modifymax(asset new_max_supply) {
    //get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(new_max_supply.symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.publisher);

    //validate
    check(reg.settings[name("modifiable")], "max supply is not modifiable");
    check(new_max_supply.is_valid(), "invalid amount");
    check(new_max_supply >= asset(0, token_sym), "max supply cannot be below zero");
    check(new_max_supply >= reg.supply, "cannot lower max supply below current supply");

    //update max supply
    registries.modify(reg, same_payer, [&](auto& col) {
        col.max_supply = new_max_supply;
    });
}

ACTION trail::setunlocker(symbol registry_sym, name new_unlock_acct, new_unlock_auth) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(registry_sym.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.publisher);

    //validate
    check(!reg.locked, "registry is locked");

    //update unlock acct and auth
    registries.modify(reg, same_payer, [&](auto& col) {
        col.unlock_acct = new_unlock_acct;
        col.unlock_auth = new_unlock_auth;
    });
}

ACTION trail::lockreg(symbol registry_sym) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(registry_sym.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.publisher);
    check(!reg.locked, "registry is already locked");

    //update lock
    registries.modify(reg, same_payer, [&](auto& col) {
        col.locked = true;
    });
}

ACTION trail::unlockreg(symbol registry_sym) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(registry_sym.code().raw(), "registry not found");

    //authenticate
    require_auth2(reg.unlock_acct, reg.unlock_auth);

    //validate
    check(reg.locked, "registry is already unlocked");

    //update lock
    registries.modify(reg, same_payer, [&](auto& col) {
        col.locked = false;
    });
}



//======================== ballot actions ========================

ACTION trail::newballot(name ballot_name, name category, name publisher, symbol token_sym, vector<name> initial_options) {
    //authenticate
    require_auth(publisher);

    //open ballots table
    ballots ballots(get_self(), get_self().value);
    auto bal = ballots.find(ballot_name.value);

    //validate
    check(bal == ballots.end(), "ballot name already exists");
    check(is_registry(token_sym), "registry not found");
    check(has_balance(publisher, token_sym), "publisher doesn't have a balance of token");

    //create initial options map
    map<name, asset> new_initial_options;

    for (name n : initial_options) {
        new_initial_options[n] = asset(0, token_sym);
    }

    //emplace new ballot
    ballots.emplace(get_self(), [&](auto& col){
        col.ballot_name = ballot_name;
        col.category = category;
        col.publisher = publisher;
        col.status = name("setup");
        col.title = "";
        col.description = "";
        col.ballot_info = "";
        col.options = new_initial_options;
        col.token_sym = token_sym;
        col.voters = 0;
        col.light_ballot = false;
        col.max_options = 1;
        col.allow_revoting = true;
        col.begin_time = time_point_sec(0);
        col.end_time = time_point_sec(0);
    });
}

ACTION trail::editdetails(name ballot_name, string title, string description, string ballot_info) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.status == name("setup"), "ballot must have setup status to edit details");

    //update ballot details
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.title = title;
        col.description = description;
        col.ballot_info = ballot_info;
    });
}

ACTION trail::editsettings(name ballot_name, bool light_ballot, uint8_t max_options, bool allow_revoting) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.status == name("setup"), "ballot must have setup status to edit settings");

    //update ballot settings
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.light_ballot = light_ballot;
        col.max_options = max_options;
        col.allow_revoting = allow_revoting;
    });
}

ACTION trail::addoption(name ballot_name, name option_name) {
    require_auth(publisher);

    //get ballot
    ballots ballots(get_self(), get_self().value);
    auto&
     bal = ballots.get(ballot_name.value, "ballot name doesn't exist");

    //validate
    check(bal.publisher == publisher, "only ballot publisher can add options");
    check(bal.status == SETUP, "ballot must be in setup mode to edit");
    check(is_option_in_ballot(option_name, bal.options) == false, "option is already in ballot");

    option new_option = {
        option_name,
        option_info,
        asset(0, bal.voting_symbol)
    };

    ballots.modify(bal, same_payer, [&](auto& row) {
        row.options.emplace_back(new_option); //TODO: maybe push_back()?
    });
}

void trail::readyballot(name ballot_name, name publisher, uint32_t end_time) {
    require_auth(publisher);

    //get ballot
    ballots ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot name doesn't exist");

    //validate
    check(bal.publisher == publisher, "only ballot publisher can ready ballot");
    check(bal.options.size() >= 2, "ballot must have at least 2 options");
    check(bal.status == SETUP, "ballot must be in setup mode to ready");
    check(end_time - now() >= MIN_BALLOT_LENGTH, "ballot must be open for at least 1 day");

    ballots.modify(bal, same_payer, [&](auto& row) {
        row.begin_time = now();
        row.end_time = end_time;
        row.status = OPEN;
    });

}

void trail::cancelballot(name ballot_name, name publisher) {
    //get ballot
    ballots ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot name doesn't exist");

    //authenticate
    require_auth(publisher);

    //validate
    check(bal.publisher == publisher, "only ballot publisher can ready ballot");
    check(bal.status == OPEN, "ballot must be in open mode to cancel");

    ballots.modify(bal, same_payer, [&](auto& row) {
        row.status = CANCELLED;
    });
}

void trail::closeballot(name ballot_name, name publisher, uint8_t new_status) {
    require_auth(publisher);

    //get ballot
    ballots ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot name doesn't exist");

    //validate
    check(bal.publisher == publisher, "only ballot publisher can ready ballot");
    check(bal.status == OPEN, "ballot must be in open mode to close");
    check(bal.end_time < now(), "must be past ballot end time to close");

    ballots.modify(bal, same_payer, [&](auto& row) {
        row.status = CLOSED;
    });
}

void trail::deleteballot(name ballot_name, name publisher) {
    //get ballot
    ballots ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot name doesn't exist");

    //authenticate
    require_auth(publisher);

    //validate
    check(bal.publisher == publisher, "only ballot publisher can delete ballot");
    check(bal.status != OPEN, "cannot delete while voting is in progress");
    check(now() > bal.end_time + BALLOT_COOLDOWN, "cannot delete until 3 days past ballot's end time");

    //TODO: add exception for polls 

    ballots.erase(bal);
}

void trail::cleanupvotes(name voter, uint16_t count, symbol voting_sym) {

    //get account
    accounts accounts(get_self(), voter.value);
    auto& acc = accounts.get(voting_sym.code().raw(), "account not found");
    
    //sort votes by expiration, lowest first
    votes votes(get_self(), voter.value);
    auto sorted_votes = votes.get_index<name("byexp")>(); 
    auto sv_itr = sorted_votes.begin();

    uint16_t votes_deleted = 0;

    //deletes expired votes until count reaches 0 or end of table, skips active votes
    while (count > 0 && sv_itr->amount.symbol == voting_sym && sv_itr != sorted_votes.end()) {
        if (sv_itr->expiration < now()) { //expired
            //print("\ncleaning vote for ", sv_itr->ballot_name);
            sv_itr = sorted_votes.erase(sv_itr); //returns next iterator
            count--;
            votes_deleted++;
        } else { //active
            sv_itr++;
        }
    }

    //update account's num_votes if votes were deleted
    accounts.modify(acc, same_payer, [&](auto& row) {
        row.num_votes -= votes_deleted;
    });

}

void trail::cleanhouse(name voter) {

    //sort votes by expiration, lowest first
    votes votes(get_self(), voter.value);
    auto sorted_votes = votes.get_index<name("byexp")>();
    auto sv_itr = sorted_votes.begin();

    //deletes all expired votes, skips active votes
    while (sv_itr != sorted_votes.end()) {
        if (sv_itr->expiration > now()) { //expired
            sv_itr = sorted_votes.erase(sv_itr); //returns next iterator
        } else { //active
            sv_itr++;
        }
    }

}

void trail::archive(name ballot_name, name publisher) {
    //get ballot
    ballots ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_recipient(publisher);
    check(bal.publisher == publisher, "only ballot publisher can archive a ballot");
    check(bal.status == CLOSED, "can only archive ballots that were properly closed");

    check(false, "***Archive Action In Development***");

    //replace ram payer with Trail
    ballots.modify(bal, name("trailservice"), [&](auto& row) {
        row.ballot_name = bal.ballot_name;
        row.category = bal.category;
        row.publisher = bal.publisher;
        row.title = bal.title;
        row.description = bal.description;
        row.info_url = bal.info_url;
        row.options = bal.options;
        row.unique_voters = bal.unique_voters;
        row.max_votable_options = bal.max_votable_options;
        row.voting_symbol = bal.voting_symbol;
        row.begin_time = bal.begin_time;
        row.end_time = bal.end_time;
        row.status = bal.status;
    });
}





//======================== voter actions ========================

void trail::regvoter(name owner, symbol token_sym) {
    //get registry
    registries registries(get_self(), get_self().value);
    auto& reg = registries.get(token_sym.code().raw(), "registry with symbol not found");

    //authenticate
    require_auth(owner);

    //check account balance doesn't already exist
    accounts accounts(get_self(), owner.value);
    auto acc = accounts.find(token_sym.code().raw());
    check(acc == accounts.end(), "account balance already exists");
    check(token_sym != symbol("TLOS", 4), "cannot open a TLOS balance here");

    //emplace account with zero balance
    accounts.emplace(owner, [&](auto& row){
        row.balance = asset(0, token_sym);
    });
}

void trail::castvote(name voter, name ballot_name, name option) {
    require_auth(voter);

    //TODO: attempt to clean up at least 2 old votes

    //get ballot
    ballots ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot name doesn't exist");
    check(now() >= bal.begin_time && now() <= bal.end_time, "must vote between ballot's begin and end time");
    check(bal.status == OPEN, "ballot status is not open for voting");

    //get account
    accounts accounts(get_self(), voter.value);
    auto& acc = accounts.get(bal.voting_symbol.code().raw(), "account balance not found");
    check(acc.num_votes < MAX_VOTE_RECEIPTS, "reached max concurrent votes for voting token");
    check(acc.balance.amount > int64_t(0), "cannot vote with a balance of 0");

    //check option exists
    int idx = get_option_index(option, bal.options);
    check(idx != -1, "option not found on ballot");

    //get votes
    votes votes(get_self(), voter.value);
    auto v_itr = votes.find(ballot_name.value);

    if (v_itr != votes.end()) { //vote for ballot exists
        
        //validate
        check(!is_option_in_receipt(option, v_itr->option_names), "voter has already voted for this option");
        check(v_itr->option_names.size() + 1 <= bal.max_votable_options, "already voted for max number of options allowed by ballot");

        //add votes to ballot option
        ballots.modify(bal, same_payer, [&](auto& row) {
            row.options[idx].votes += acc.balance;
        });

        //update vote with new option name
        votes.modify(v_itr, same_payer, [&](auto& row) {
            row.option_names.emplace_back(option);
        });

    } else { //vote doesn't already exist

        //validate

        vector<name> new_option_names;
        new_option_names.emplace_back(option);

        //emplace new vote
        votes.emplace(voter, [&](auto& row) {
            row.ballot_name = ballot_name;
            row.option_names = new_option_names;
            row.amount = acc.balance;
            row.expiration = bal.end_time;
        });
        
        //add votes to ballot option
        ballots.modify(bal, same_payer, [&](auto& row) {
            row.options[idx].votes += acc.balance;
            row.unique_voters += 1;
        });

        //update num_votes on account
        accounts.modify(acc, same_payer, [&](auto& row) {
            row.num_votes += 1;
        });

    }
}

void trail::unvote(name voter, name ballot_name, name option) {
    require_auth(voter);

    //get ballot
    ballots ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot name doesn't exist");

    //get votes
    votes votes(get_self(), voter.value);
    auto& v = votes.get(ballot_name.value, "vote does not exist for this ballot");

    //get account
    accounts accounts(get_self(), voter.value);
    auto& acc = accounts.get(bal.voting_symbol.code().raw(), "account balance not found");

    auto bal_opt_idx = get_option_index(option, bal.options);
    auto new_voted_options = v.option_names;
    bool found = false;

    for (auto opt_itr = new_voted_options.begin(); opt_itr < new_voted_options.end(); opt_itr++) {
        if (*opt_itr == option) {
            new_voted_options.erase(opt_itr);
            found = true;
            break;
        }
    }

    //validate
    check(bal.status == OPEN, "ballot status is not open for voting");
    check(now() >= bal.begin_time && now() <= bal.end_time, "must unvote between ballot's begin and end time");
    check(found, "option not found on vote");
    check(bal_opt_idx != -1, "option not found on ballot");

    if (new_voted_options.size() > 0) { //votes for ballot still remain

        auto new_bal_options = bal.options;
        new_bal_options[bal_opt_idx].votes -= v.amount;
        
        //remove option from vote
        votes.modify(v, same_payer, [&](auto& row) {
            row.option_names = new_voted_options;
        });

        //lower option votes by amount
        ballots.modify(bal, same_payer, [&](auto& row) {
            row.options = new_bal_options;
        });

    } else { //unvoted last option

        //erase vote
        votes.erase(v);

        //decrement bal.unique_voters;
        ballots.modify(bal, same_payer, [&](auto& row) {
            row.unique_voters -= 1;
        });

        //decrement num_votes on account
        accounts.modify(acc, same_payer, [&](auto& row) {
            row.num_votes -= 1;
        });

    }

}

void trail::rebalance(name voter) {

    /*currently rebalances only apply to TLOS/VOTE
    action will fail if it can't rebalance all votes within max trx time (currently capped at 51) */

    //get current tlos stake, convert to VOTE
    auto current_stake = get_staked_tlos(voter);
    auto vote_stake = asset(current_stake.amount, VOTE_SYM);

    //get current VOTE account
    accounts accounts(get_self(), voter.value);
    auto& acc = accounts.get(VOTE_SYM.code().raw(), "account not found");

    //calc vote delta (new - old)
    asset delta = vote_stake - acc.balance;

    //start at beginning of votes table
    votes votes(get_self(), voter.value);
    auto v_itr = votes.begin();

    //updates all relevant votes with rebalance delta
    while (v_itr != votes.end()) {

        bool did_rebalance = applied_rebalance(v_itr->ballot_name, delta, v_itr->option_names);

        if (did_rebalance) {
            //update vote
            votes.modify(v_itr, same_payer, [&](auto& row) {
                row.amount += delta;
            });
        }

        v_itr++;
    }

    //update account with new balance
    accounts.modify(acc, same_payer, [&](auto& row) {
        row.balance = vote_stake;
    });

}

void trail::unregvoter(name owner, symbol token_sym) {
    //get account
    accounts accounts(get_self(), owner.value);
    auto& acc = accounts.get(token_sym.code().raw(), "account balance doesn't exist");

    //authenticate
    require_auth(owner);

    //validate
    check(acc.balance == asset(0, token_sym), "cannot close an account still holding tokens");

    accounts.erase(acc);
}

//========== functions ==========

//add quantity of tokens to balance
void trail::add_balance(name owner, asset quantity) {
    accounts_table to_accts(get_self(), owner.value);
    auto& to_acct = to_accts.get(quantity.symbol.code().raw(), "add_balance: account not found");
    
    to_accts.modify(to_acct, same_payer, [&](auto& col) {
        col.balance += quantity;
    });
}

//subtract quantity of tokens from balance
void trail::sub_balance(name owner, asset quantity) {
    accounts_table from_accts(get_self(), owner.value);
    auto& from_acct = from_accts.get(quantity.symbol.code().raw(), "sub_balance: account not found");

    check(from_acct.balance >= quantity, "overdrawn balance");
    
    from_accts.modify(from_acct, same_payer, [&](auto& col) {
        col.balance -= quantity;
    });
}



//
bool trail::has_token_balance(name voter, symbol sym) {
    accounts accounts(get_self(), voter.value);
    auto a_itr = accounts.find(sym.code().raw());
    if (a_itr != accounts.end()) {
        return true;
    }
    return false;
}

asset trail::get_staked_tlos(name owner) {
    user_resources_table userres(name("eosio"), owner.value);
    auto r = userres.find(owner.value);

    int64_t amount = 0;

    if (r != userres.end()) {
        auto res = *r;
        amount = (res.cpu_weight.amount + res.net_weight.amount);
    }
    
    return asset(amount, symbol("TLOS", 4));
}

bool trail::applied_rebalance(name ballot_name, asset delta, vector<name> options_to_rebalance) {
    ballots ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot name doesn't exist");

    //if expired or not VOTE, skip (rebalance doesn't erase, only recalcs votes to save CPU)
    if (bal.voting_symbol != VOTE_SYM || now() > bal.end_time) {
        return false;
    }

    auto bal_ops = bal.options;

    //loop over options_to_rebalance, rebalance each
    for (auto i = options_to_rebalance.begin(); i < options_to_rebalance.end(); i++) {
        //get option index on ballot
        auto bal_opt_idx = get_option_index(*i, bal.options);
        //update option with vote delta
        bal_ops[bal_opt_idx].votes += delta;
    }

    //apply rebalance to ballot
    ballots.modify(bal, same_payer, [&](auto& row) {
        row.options = bal_ops;
    });

    return true;
}



//========== dispatcher ==========

extern "C"
{
    void apply(uint64_t receiver, uint64_t code, uint64_t action)
    {
        size_t size = action_data_size();
        constexpr size_t max_stack_buffer_size = 512;
        void* buffer = nullptr;
        if( size > 0 ) {
            buffer = max_stack_buffer_size < size ? malloc(size) : alloca(size);
            read_action_data(buffer, size);
        }
        datastream<const char*> ds((char*)buffer, size);

        if (code == receiver)
        {
            switch (action)
            {
                EOSIO_DISPATCH_HELPER(trail, 
                    (newballot)(upsertinfo)(addoption)(readyballot)(cancelballot)(closeballot)(deleteballot)(archive)
                    (newtoken)(mint)(burn)(send)(seize)(changemax)
                    (regvoter)(castvote)(unvote)(rebalance)(cleanupvotes)(cleanhouse)(unregvoter));
            }

        } else if (code == name("eosio").value && action == name("undelegatebw").value) {

            // struct undelegatebw_args {
            //     name from;
            //     name receiver;
            //     asset unstake_net_quantity;
            //     asset unstake_cpu_quantity;
            // };
            
            //trail trailservice(name(receiver), name(code), ds);
            //auto args = unpack_action_data<undelegatebw_args>();
            //trailservice.rebalance(args.from);
            execute_action<trail>(eosio::name(receiver), eosio::name(code), &trail::rebalance);
        }
    }
}