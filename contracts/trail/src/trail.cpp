#include "../include/trail.hpp"

trail::trail(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

trail::~trail() {}

//======================== ballot actions ========================

void trail::newballot(name ballot_name, name category, name publisher, 
    string title, string description, string info_url, 
    uint8_t max_votable_options, symbol voting_sym) 
{
    require_auth(publisher);

    //check ballot doesn't already exist
    ballots ballots(get_self(), get_self().value);
    auto b = ballots.find(ballot_name.value);
    check(b == ballots.end(), "ballot name already exists");
    check(max_votable_options > 0, "max votable options must be greater than 0");

    //check category is in supported list

    vector<option> blank_options;

    ballots.emplace(publisher, [&](auto& row){
        row.ballot_name = ballot_name;
        row.category = category;
        row.publisher = publisher;
        row.title = title;
        row.description = description;
        row.info_url = info_url;
        row.options = blank_options;
        row.unique_voters = 0;
        row.max_votable_options = max_votable_options;
        row.voting_symbol = voting_sym;
        row.begin_time = 0;
        row.end_time = 0;
        row.status = SETUP;
    });

}

void trail::upsertinfo(name ballot_name, name publisher, string title, string description,
    string info_url, uint8_t max_votable_options) {
    //get ballot
    ballots ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot name doesn't exist");

    //authenticate
    require_auth(publisher);
    check(bal.publisher == publisher, "only ballot publisher can set info");

    //validate
    check(bal.status == SETUP, "ballot must be in setup mode to edit");

    ballots.modify(bal, same_payer, [&](auto& row) {
        row.title = title;
        row.description = description;
        row.info_url = info_url;
        row.max_votable_options = max_votable_options;
    });

}

void trail::addoption(name ballot_name, name publisher, name option_name, string option_info) {
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

//======================== token actions ========================

void trail::newtoken(name publisher, asset max_supply, token_settings settings, string info_url) {
    //authenticate
    require_auth(publisher);

    symbol new_sym = max_supply.symbol;

    //check registry doesn't already exist
    registries registries(get_self(), get_self().value);
    auto reg = registries.find(new_sym.code().raw());
    check(reg == registries.end(), "registry with symbol not found");
    check(new_sym.code().raw() != symbol("TLOS", 4).code().raw(), "the TLOS symbol is restricted to avoid confusion with the system token");

    registries.emplace(publisher, [&](auto& row){
        row.supply = asset(0, new_sym);
        row.max_supply = max_supply;
        row.publisher = publisher;
        row.total_voters = uint32_t(0);
        row.total_proxies = uint32_t(0);
        row.settings = settings;
        row.info_url = info_url;
    });

}

void trail::mint(name publisher, name recipient, asset amount_to_mint) {
    symbol token_sym = amount_to_mint.symbol;

    //get registry
    registries registries(get_self(), get_self().value);
    auto& reg = registries.get(token_sym.code().raw(), "registry with symbol not found");

    //get account
    accounts accounts(get_self(), recipient.value);
    auto& acc = accounts.get(token_sym.code().raw(), "account balance not found");

    //authenticate
    require_auth(publisher);
    check(reg.publisher == publisher, "only registry publisher is authorized to mint tokens");

    //validate
    check(is_account(recipient), "recipient account doesn't exist");
    check(reg.publisher == publisher, "only registry publisher can mint new tokens");
    check(reg.supply + amount_to_mint <= reg.max_supply, "cannot mint tokens beyond max_supply");
    check(amount_to_mint > asset(0, token_sym), "must mint a positive amount");
    check(amount_to_mint.is_valid(), "invalid amount");

    //update recipient balance
    accounts.modify(acc, same_payer, [&](auto& row) {
        row.balance += amount_to_mint;
    });

    //update registry supply
    registries.modify(reg, same_payer, [&](auto& row) {
        row.supply += amount_to_mint;
    });

}

void trail::burn(name publisher, asset amount_to_burn) {
    symbol token_sym = amount_to_burn.symbol;

    //get registry
    registries registries(get_self(), get_self().value);
    auto& reg = registries.get(token_sym.code().raw(), "registry with symbol not found");

    //get account
    accounts accounts(get_self(), publisher.value);
    auto& acc = accounts.get(token_sym.code().raw(), "account balance not found");

    //authenticate
    require_auth(publisher);
    check(reg.publisher == publisher, "only registry publisher is authorized to burn tokens");

    //validate
    check(reg.settings.is_burnable, "token is not burnable");
    check(reg.supply - amount_to_burn >= asset(0, token_sym), "cannot burn more tokens than exist");
    check(acc.balance >= amount_to_burn, "cannot burn more tokens than owned");
    check(amount_to_burn > asset(0, token_sym), "must burn a positive amount");
    check(amount_to_burn.is_valid(), "invalid amount");

    //update publisher balance
    accounts.modify(acc, same_payer, [&](auto& row) {
        row.balance -= amount_to_burn;
    });
}

void trail::send(name sender, name recipient, asset amount, string memo) {
    //get registry
    registries registries(get_self(), get_self().value);
    auto& reg = registries.get(amount.symbol.code().raw(), "registry with symbol not found");

    //authenticate
    require_auth(sender);

    //validate
    check(reg.settings.is_liquid, "token is not liquid and cannot be sent");
    check(sender != recipient, "cannot send tokens to yourself");
    check(is_account(recipient), "recipient account doesn't exist");
    check(amount.is_valid(), "invalid amount");
    check(amount.amount > 0, "must transfer positive amount");
    check(amount.symbol == reg.max_supply.symbol, "symbol precision mismatch");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //sub amount from sender
    sub_balance(sender, amount);

    //add amount to recipient
    add_balance(recipient, amount);

    //TODO: trigger rebalance?
}

void trail::seize(name publisher, name owner, asset amount_to_seize) {
    symbol token_sym = amount_to_seize.symbol;

    //get registry
    registries registries(get_self(), get_self().value);
    auto& reg = registries.get(token_sym.code().raw(), "registry with symbol not found");

    //authenticate
    require_auth(publisher);
    check(reg.publisher == publisher, "only registry publisher is authorized to seize tokens");

    //validate
    check(reg.settings.is_seizable, "token is not seizable");
    check(publisher != owner, "cannot seize tokens from yourself");
    check(is_account(owner), "owner account doesn't exist");
    check(amount_to_seize.is_valid(), "invalid amount");
    check(amount_to_seize.amount > 0, "must seize positive amount");
    check(amount_to_seize.symbol == reg.max_supply.symbol, "symbol precision mismatch");

    //sub amount from owner
    sub_balance(owner, amount_to_seize);

    //add amount to publisher
    add_balance(publisher, amount_to_seize);
}

void trail::changemax(name publisher, asset max_supply_delta) {
    symbol token_sym = max_supply_delta.symbol;

    //get registry
    registries registries(get_self(), get_self().value);
    auto& reg = registries.get(token_sym.code().raw(), "registry with symbol not found");

    //authenticate
    require_auth(publisher);
    check(reg.publisher == publisher, "only registry publisher is authorized to seize tokens");

    //validate
    check(reg.settings.is_max_mutable, "token's max supply is not mutable");
    check(max_supply_delta.is_valid(), "invalid amount");
    check(max_supply_delta.symbol == reg.max_supply.symbol, "symbol precision mismatch");
    check(reg.max_supply - max_supply_delta >= asset(0, token_sym), "cannot lower max_supply below zero");
    check(reg.max_supply - max_supply_delta >= reg.supply, "cannot lower max_supply below circulating supply");

    //change max
    registries.modify(reg, same_payer, [&](auto& row) {
        row.max_supply += max_supply_delta;
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

bool trail::is_option_in_ballot(name option_name, vector<option> options) {
    for (option opt : options) {
        if (option_name == opt.option_name) {
            return true;
        }
    }

    return false;
}

bool trail::is_option_in_receipt(name option_name, vector<name> options_voted) {
    return std::find(options_voted.begin(), options_voted.end(), option_name) != options_voted.end();
}

int trail::get_option_index(name option_name, vector<option> options) {
    for (int i = 0; i < options.size(); i++) {
        if (option_name == options[i].option_name) {
            return i;
        }
    }

    return -1;
}

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

void trail::add_balance(name owner, asset amount) {
    accounts to_accts(get_self(), owner.value);
    auto&
    to_acct = to_accts.get(amount.symbol.code().raw(), "recipient account not found");
    
    to_accts.modify(to_acct, same_payer, [&](auto& row) {
        row.balance += amount;
    });
}

void trail::sub_balance(name owner, asset amount) {
    accounts from_accts(get_self(), owner.value);
    auto& from_acct = from_accts.get(amount.symbol.code().raw(), "sender account not found");
    
    from_accts.modify(from_acct, same_payer, [&](auto& row) {
        row.balance -= amount;
    });
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