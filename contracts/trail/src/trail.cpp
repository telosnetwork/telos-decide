#include "../include/trail.hpp"

trail::trail(name self, name code, datastream<const char*> ds) : contract(self, code, ds) {}

trail::~trail() {}

//======================== admin actions ========================

ACTION trail::setconfig(string trail_version, asset ballot_fee, asset registry_fee, asset archival_fee,
    uint32_t min_ballot_length, uint32_t ballot_cooldown, uint16_t max_vote_receipts) {
    //authenticate
    require_auth(get_self());

    //recommended units
    // uint32_t MIN_BALLOT_LENGTH = 86400; //1 day
    // uint16_t MAX_VOTE_RECEIPTS = 51;
    // uint32_t BALLOT_COOLDOWN = 432000;  //5 days in seconds

    //recommended fees
    // asset BALLOT_LISTING_FEE = asset(150000, TLOS_SYM);
    // asset REGISTRY_LISTING_FEE = asset(250000, TLOS_SYM);
    // asset ARCHIVAL_BASE_FEE = asset(50000, TLOS_SYM);

    //open config singleton
    config_singleton configs(get_self(), get_self().value);

    //build new configs
    config new_config = {
        trail_version, //trail_version
        ballot_fee, //ballot_listing_fee
        registry_fee, //registry_creation_fee
        archival_fee, //archival_base_fee
        min_ballot_length, //min_ballot_length
        ballot_cooldown, //ballot_cooldown
        max_vote_receipts //max_vote_receipts
    };

    //set new config
    configs.set(new_config, get_self());
}

//======================== registry actions ========================

ACTION trail::newregistry(name manager, asset max_supply, name access) {
    //authenticate
    require_auth(manager);

    //open registries table, search for registry
    registries_table registries(get_self(), get_self().value);
    auto reg = registries.find(max_supply.symbol.code().raw());

    //validate
    check(reg == registries.end(), "registry already exists");
    check(max_supply.amount > 0, "max supply must be greater than 0");
    check(max_supply.symbol.is_valid(), "invalid symbol name");
    check(max_supply.is_valid(), "invalid max supply");
    check(valid_access_method(access), "invalid access method");

    //check reserved symbols
    if (manager != get_self()) {
        check(max_supply.symbol.code().raw() != TLOS_SYM.code().raw(), "TLOS symbol is reserved");
        check(max_supply.symbol.code().raw() != VOTE_SYM.code().raw(), "VOTE symbol is reserved");
        check(max_supply.symbol.code().raw() != TRAIL_SYM.code().raw(), "TRAIL symbol is reserved");
    }

    //set up initial settings
    map<name, bool> initial_settings;
    initial_settings[name("transferable")] = false;
    initial_settings[name("burnable")] = false;
    initial_settings[name("reclaimable")] = false;
    initial_settings[name("stakeable")] = false;
    initial_settings[name("unstakeable")] = false;
    initial_settings[name("maxmutable")] = false;

    //emplace new token registry, RAM paid by manager
    registries.emplace(manager, [&](auto& col) {
        col.supply = asset(0, max_supply.symbol);
        col.max_supply = max_supply;
        col.voters = uint32_t(0);
        col.access = access;
        col.locked = false;
        col.unlock_acct = manager;
        col.unlock_auth = name("active");
        col.manager = manager;
        col.settings = initial_settings;
        col.open_ballots = uint16_t(0);
        col.rebalanced_volume = asset(0, max_supply.symbol);
        col.rebalanced_count = uint32_t(0);
    });

}

ACTION trail::togglereg(symbol registry_symbol, name setting_name) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(registry_symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.manager);

    //validate
    check(!reg.locked, "registry is locked");
    auto set_itr = reg.settings.find(setting_name);
    check(set_itr != reg.settings.end(), "setting not found");

    //update setting
    registries.modify(reg, same_payer, [&](auto& col) {
        col.settings[setting_name] = !reg.settings.at(setting_name);
    });
}

ACTION trail::mint(name to, asset quantity, string memo) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(quantity.symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.manager);

    //validate
    check(is_account(to), "to account doesn't exist");
    check(reg.supply + quantity <= reg.max_supply, "minting would breach max supply");
    check(quantity.amount > 0, "must mint a positive quantity");
    check(quantity.is_valid(), "invalid quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //update recipient balance
    add_balance(to, quantity);

    //update registry supply
    registries.modify(reg, same_payer, [&](auto& col) {
        col.supply += quantity;
    });

    //notify to account
    require_recipient(to);
}

ACTION trail::transfer(name from, name to, asset quantity, string memo) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(quantity.symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(from);

    //validate
    check(is_account(to), "to account doesn't exist");
    check(reg.settings.at(name("transferable")), "token is not transferable");
    check(from != to, "cannot transfer tokens to yourself");
    check(quantity.amount > 0, "must transfer positive quantity");
    check(quantity.is_valid(), "invalid quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //subtract quantity from sender balance
    sub_balance(from, quantity);

    //add quantity to recipient balance
    add_balance(to, quantity);

    //notify from and to accounts
    require_recipient(from);
    require_recipient(to);
}

ACTION trail::burn(asset quantity, string memo) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(quantity.symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.manager);

    //open accounts table, get manager account
    accounts_table accounts(get_self(), reg.manager.value);
    auto& mgr_acct = accounts.get(quantity.symbol.code().raw(), "account not found");

    //validate
    check(reg.settings.at(name("burnable")), "token is not burnable");
    check(reg.supply - quantity >= asset(0, quantity.symbol), "cannot burn supply below zero");
    check(mgr_acct.balance >= quantity, "burning would overdraw balance");
    check(quantity.amount > 0, "must burn a positive quantity");
    check(quantity.is_valid(), "invalid quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //subtract quantity from manager balance
    sub_balance(reg.manager, quantity);

    //update registry supply
    registries.modify(reg, same_payer, [&](auto& col) {
        col.supply -= quantity;
    });

    //notify manager account
    require_recipient(reg.manager);
}

ACTION trail::reclaim(name voter, asset quantity, string memo) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(quantity.symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.manager);

    //validate
    check(reg.settings.at(name("reclaimable")), "token is not reclaimable");
    check(reg.manager != voter, "cannot reclaim tokens from yourself");
    check(is_account(voter), "voter account doesn't exist");
    check(quantity.is_valid(), "invalid amount");
    check(quantity.amount > 0, "must reclaim positive amount");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //sub quantity from voter balance
    sub_balance(voter, quantity);

    //add quantity to manager balance
    add_balance(reg.manager, quantity);

    //notify voter account
    require_recipient(voter);
}

ACTION trail::mutatemax(asset new_max_supply, string memo) {
    //get registries table, open registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(new_max_supply.symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.manager);

    //validate
    check(reg.settings.at(name("maxmutable")), "max supply is not modifiable");
    check(new_max_supply.is_valid(), "invalid amount");
    check(new_max_supply.amount >= 0, "max supply cannot be below zero");
    check(new_max_supply >= reg.supply, "cannot lower max supply below current supply");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //update max supply
    registries.modify(reg, same_payer, [&](auto& col) {
        col.max_supply = new_max_supply;
    });
}

ACTION trail::setunlocker(symbol registry_symbol, name new_unlock_acct, name new_unlock_auth) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(registry_symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.manager);

    //validate
    check(!reg.locked, "registry is locked");
    check(is_account(new_unlock_acct), "unlock account doesn't exist");

    //update unlock acct and auth
    registries.modify(reg, same_payer, [&](auto& col) {
        col.unlock_acct = new_unlock_acct;
        col.unlock_auth = new_unlock_auth;
    });
}

ACTION trail::lockreg(symbol registry_symbol) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(registry_symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(reg.manager);
    check(!reg.locked, "registry is already locked");

    //update lock
    registries.modify(reg, same_payer, [&](auto& col) {
        col.locked = true;
    });
}

ACTION trail::unlockreg(symbol registry_symbol) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(registry_symbol.code().raw(), "registry not found");

    //authenticate
    require_auth(permission_level{reg.unlock_acct, reg.unlock_auth});

    //validate
    check(reg.locked, "registry is already unlocked");

    //update lock
    registries.modify(reg, same_payer, [&](auto& col) {
        col.locked = false;
    });
}

//======================== ballot actions ========================

ACTION trail::newballot(name ballot_name, name category, name publisher,  
    symbol registry_symbol, name voting_method, vector<name> initial_options) {
    //authenticate
    require_auth(publisher);

    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(registry_symbol.code().raw(), "registry not found");

    //open accounts table, get account
    accounts_table accounts(get_self(), publisher.value);
    auto& acct = accounts.get(registry_symbol.code().raw(), "account not found");

    //open ballots table
    ballots_table ballots(get_self(), get_self().value);
    auto bal = ballots.find(ballot_name.value);

    //charge ballot listing fee to publisher
    require_fee(publisher, conf.ballot_listing_fee);

    //TODO: update open_ballots on registry

    //validate
    check(bal == ballots.end(), "ballot name already exists");
    check(valid_category(category), "invalid category");
    check(valid_voting_method(voting_method), "invalid voting method");

    //create initial options map, initial settings map
    map<name, asset> new_initial_options;
    map<name, bool> new_settings;

    //TODO: check initial options doesn't have duplicates
    for (name n : initial_options) {
        new_initial_options[n] = asset(0, registry_symbol);
    }

    //intitial settings
    new_settings[name("lightballot")] = false;
    new_settings[name("revotable")] = true;
    new_settings[name("votestake")] = true;

    //emplace new ballot
    ballots.emplace(publisher, [&](auto& col){
        col.ballot_name = ballot_name;
        col.category = category;
        col.publisher = publisher;
        col.status = name("setup");
        col.title = "";
        col.description = "";
        col.ballot_info = "";
        col.voting_method = voting_method;
        col.max_options = 1;
        col.options = new_initial_options;
        col.registry_symbol = registry_symbol;
        col.total_votes = asset(0, registry_symbol);
        col.total_voters = 0;
        col.settings = new_settings;
        col.cleaned_volume = asset(0, registry_symbol);
        col.cleaned_count = 0;
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
    check(bal.status == name("setup"), "ballot must be in setup mode to edit details");

    //update ballot details
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.title = title;
        col.description = description;
        col.ballot_info = ballot_info;
    });
}

ACTION trail::togglebal(name ballot_name, name setting_name) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    auto set_itr = bal.settings.find(setting_name);

    //validate
    check(bal.status == name("setup"), "ballot must be in setup mode to toggle settings");
    check(set_itr != bal.settings.end(), "setting not found");

    //update ballot settings
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.settings[setting_name] = !bal.settings.at(setting_name);
    });
}

ACTION trail::editmaxopts(name ballot_name, uint8_t new_max_options) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.status == name("setup"), "ballot must be in setup mode to edit max options");
    check(new_max_options > 0, "max options must be greater than zero");
    check(new_max_options <= bal.options.size(), "max options cannot be greater than number of options");

    //update ballot settings
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.max_options = new_max_options;
    });
}

ACTION trail::addoption(name ballot_name, name new_option_name) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.status == name("setup"), "ballot must be in setup mode to add options");
    check(bal.options.find(new_option_name) == bal.options.end(), "option is already in ballot");

    ballots.modify(bal, same_payer, [&](auto& col) {
        col.options[new_option_name] = asset(0, bal.registry_symbol);
    });
}

ACTION trail::rmvoption(name ballot_name, name option_name) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //find option in ballot
    auto opt_itr = bal.options.find(option_name);

    //validate
    check(bal.status == name("setup"), "ballot must be in setup mode to remove options");
    check(opt_itr != bal.options.end(), "option not found");

    //erase ballot option
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.options.erase(opt_itr);
    });
}

ACTION trail::readyballot(name ballot_name, time_point_sec end_time) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //initialize
    auto now = time_point_sec(current_time_point());

    //open config singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //validate
    check(bal.options.size() >= 2, "ballot must have at least 2 options");
    check(bal.status == name("setup"), "ballot must be in setup mode to ready");
    check(end_time.sec_since_epoch() - now.sec_since_epoch() >= conf.min_ballot_length, "ballot must be open for at least 1 day");
    check(end_time.sec_since_epoch() > now.sec_since_epoch(), "end time must be in the future");

    ballots.modify(bal, same_payer, [&](auto& col) {
        col.status = name("voting");
        col.begin_time = now;
        col.end_time = end_time;
    });
}

ACTION trail::cancelballot(name ballot_name, string memo) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.status == name("voting"), "ballot must be in voting mode to cancel");

    ballots.modify(bal, same_payer, [&](auto& col) {
        col.status = name("cancelled");
    });
}

ACTION trail::deleteballot(name ballot_name) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //open configs singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //authenticate
    require_auth(bal.publisher);

    //initialize
    auto now = time_point_sec(current_time_point());

    //validate
    check(bal.status != name("voting"), "cannot delete while voting is in progress");
    check(bal.status != name("archived"), "cannot delete archived ballot");
    check(now > bal.end_time + conf.ballot_cooldown, "cannot delete until 5 days past ballot's end time");

    //TODO: check all votes are cleaned?

    //erase ballot
    ballots.erase(bal);
}

ACTION trail::closeballot(name ballot_name, bool post_results) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.status == name("voting"), "ballot must be in voting mode to close");
    check(bal.end_time < time_point_sec(current_time_point()), "must be past ballot end time to close");

    //change ballot status
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.status = name("closed");
    });

    //TODO: perform 1tokensquare1v final sqrt() here

    //if post_results true, send postresults inline to self
    if (post_results) {
        action(permission_level{get_self(), name("active")}, name("trailservice"), name("postresults"), make_tuple(
            ballot_name, //ballot_name
            bal.options, //results
            bal.voting_method, //voting_method
            bal.total_votes, //total_votes
            bal.total_voters //total_voters
        )).send();
    }
}

ACTION trail::postresults(name ballot_name, map<name, asset> final_results, 
    name voting_method, asset total_votes, uint32_t total_voters) {

    //authenticate
    //TODO: require_auth(permission_level{get_self(), name("postresults")});
    require_auth(get_self());

    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value);

    //initialize
    auto now = time_point_sec(current_time_point());

    //validate
    check(now > bal.end_time, "ballot voting must be over before posting results");

    //notify ballot publisher (for external contract processing)
    require_recipient(bal.publisher);
}

ACTION trail::archive(name ballot_name, time_point_sec archived_until) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.status == name("closed"), "ballot must be closed to archive");

    //TODO: charge user

    //TODO: place in archived table

    //change ballot status to archived
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.status = name("archived");
    });
}

ACTION trail::unarchive(name ballot_name) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //change ballot status to closed
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.status = name("closed");
    });
}

//======================== voter actions ========================

ACTION trail::regvoter(name voter, symbol registry_symbol, optional<name> referrer) {
    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(registry_symbol.code().raw(), "registry not found");

    //check account balance doesn't already exist
    accounts_table accounts(get_self(), voter.value);
    auto acct = accounts.find(registry_symbol.code().raw());

    //validate
    check(acct == accounts.end(), "voter already exists");
    check(registry_symbol != symbol("TLOS", 4), "cannot register as TLOS voter, use VOTE instead");

    //initialize
    name ram_payer = voter;

    //authenticate
    switch (reg.access.value) {
        case (name("public").value):
            require_auth(voter);
            break;
        case (name("private").value):
            require_auth(reg.manager);
            break;
        case (name("invite").value):
            if (referrer) {
                name ref = *referrer;
                require_auth(ref);
                //TODO: check referrer has a balance
                ram_payer = ref;
            }
            break;
        case (name("membership").value):
            //inline sent from trailservice@membership
            require_auth(permission_level{get_self(), name("membership")});
            ram_payer = get_self();
            break;
        default:
            check(false, "invalid access method. contact registry manager.");
    }

    //emplace account with zero balance
    accounts.emplace(ram_payer, [&](auto& col) {
        col.balance = asset(0, registry_symbol);
        col.staked = asset(0, registry_symbol);
    });

    //update registry
    registries.modify(reg, same_payer, [&](auto& col) {
        col.voters += 1;
    });
}

ACTION trail::unregvoter(name voter, symbol registry_symbol) {
    //authenticate
    require_auth(voter);

    //open accounts table, get account
    accounts_table accounts(get_self(), voter.value);
    auto& acct = accounts.get(registry_symbol.code().raw(), "account not found");

    //validate
    check(acct.balance == asset(0, registry_symbol), "cannot unregister unless balance is zero");
    check(acct.staked == asset(0, registry_symbol), "cannot unregister unless staked is zero");

    //TODO: let voter unregister anyway by sending liquid and staked amount?

    //TODO: require voter to cleanup/unvote all existing votes first?

    //erase account
    accounts.erase(acct);
}

ACTION trail::castvote(name voter, name ballot_name, vector<name> options) {
    //authenticate
    require_auth(voter);

    //TODO: first attempt to clean up 1 old vote

    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //open accounts table, get account
    accounts_table accounts(get_self(), voter.value);
    auto& acct = accounts.get(bal.registry_symbol.code().raw(), "account not found");

    //open votes table, search for existing vote
    votes_table votes(get_self(), voter.value);
    auto v_itr = votes.find(ballot_name.value);

    //initialize
    auto now = time_point_sec(current_time_point());
    asset raw_vote_weight = asset(0, bal.registry_symbol);
    uint32_t new_voter = 1;
    map<name, asset> temp_bal_options = bal.options;

    if (bal.settings.at(name("votestake"))) { //use stake
        raw_vote_weight = acct.staked;
    } else { //use balance
        raw_vote_weight = acct.balance;
    }

    //validate
    check(bal.status == name("voting"), "ballot status is must be in voting mode to cast vote");
    check(now >= bal.begin_time && now <= bal.end_time, "vote must occur between ballot begin and end times");
    check(options.size() <= bal.max_options, "cannot vote for more than ballot's max options");
    check(raw_vote_weight.amount > 0, "must vote with a positive amount");
    //TODO: check all voter's options exist in ballot

    //rollback if vote already exists
    if (v_itr != votes.end()) {
        //initialize
        auto v = *v_itr;

        //validate
        check(bal.settings.at(name("revotable")), "ballot is not revotable");
        
        //rollback old vote
        for (auto i = v.options_voted.begin(); i != v.options_voted.end(); i++) {
            //TODO: check option exists to rollback
            //TODO: rollback in order of weight so ranked votes aren't corrupted

            temp_bal_options[i->first] -= i->second;
        }

        //update
        new_voter = 0;
    }

    //calc new vote mapping
    auto new_votes = calc_vote_mapping(bal.registry_symbol, bal.voting_method, 
    options, raw_vote_weight);

    //apply new votes to ballot options
    for (auto i = new_votes.begin(); i != new_votes.end(); i++) {
        temp_bal_options[i->first] += i->second;
    }

    //update ballot
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.options = temp_bal_options;
        col.total_voters += new_voter;
    });

    //update existing votes, or emplace votes if new
    if (new_voter == 1) {
        votes.emplace(voter, [&](auto& col) {
            col.ballot_name = ballot_name;
            col.registry_symbol = bal.registry_symbol;
            col.options_voted = new_votes;
            col.expiration = bal.end_time;
        });
    } else {
        //update votes
        votes.modify(v_itr, same_payer, [&](auto& col) {
            col.options_voted = new_votes;
        });
    }

}

ACTION trail::unvoteall(name voter, name ballot_name) {
    //authenticate
    require_auth(voter);

    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //open votes table, get vote
    votes_table votes(get_self(), voter.value);
    auto& v = votes.get(ballot_name.value, "vote not found");

    //open accounts table, get account
    accounts_table accounts(get_self(), voter.value);
    auto& acct = accounts.get(bal.registry_symbol.code().raw(), "account not found");

    //initialize
    map<name, asset> temp_bal_options = bal.options;
    auto now = time_point_sec(current_time_point());

    //validate
    check(bal.status == name("voting"), "ballot must be in voting mode to unvote");
    check(now >= bal.begin_time && now <= bal.end_time, "must unvote between begin and end time");

    //rollback current options voted
    for (auto i = v.options_voted.begin(); i != v.options_voted.end(); i++) {
        //TODO: check voted option still exists on ballot

        temp_bal_options[i->first] -= i->second;
    }

    //update ballot
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.options = temp_bal_options;
        col.total_voters -= 1;
    });

    //erase old votes
    votes.erase(v);
    
}

ACTION trail::stake(name voter, asset quantity) {
    //authenticate
    require_auth(voter);

    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(quantity.symbol.code().raw(), "registry not found");

    //validate
    check(reg.settings.at(name("stakeable")), "token is not stakeable");
    check(is_account(voter), "voter account doesn't exist");
    check(quantity.is_valid(), "invalid amount");
    check(quantity.amount > 0, "must stake positive amount");

    //subtract quantity from liquid balance
    sub_balance(voter, quantity);

    //add quantity to staked balance
    add_stake(voter, quantity);
}

ACTION trail::unstake(name voter, asset quantity) {
    //authenticate
    require_auth(voter);

    //open registries table, get registry
    registries_table registries(get_self(), get_self().value);
    auto& reg = registries.get(quantity.symbol.code().raw(), "registry not found");

    //validate
    check(reg.settings.at(name("unstakeable")), "token is not unstakeable");
    check(is_account(voter), "voter account doesn't exist");
    check(quantity.is_valid(), "invalid amount");
    check(quantity.amount > 0, "must unstake positive amount");

    //subtract quantity from stake
    sub_stake(voter, quantity);

    //add quantity to balance
    add_balance(voter, quantity);
}

//======================== worker actions ========================

ACTION trail::regworker(name worker_name) {
    //authenticate
    require_auth(worker_name);

    //open workers table, get worker
    workers_table workers(get_self(), get_self().value);
    auto wrk = workers.find(worker_name.value);

    //validate
    check(wrk == workers.end(), "worker already registered");

    //initialize
    auto now = time_point_sec(current_time_point());
    map<symbol, asset> new_rebalance_volume;
    map<symbol, uint16_t> new_rebalance_count;
    map<name, asset> new_clean_volume;
    map<name, uint16_t> new_clean_count;

    //register new worker
    workers.emplace(worker_name, [&](auto& col) {
        col.worker_name = worker_name;
        col.standing = name("good");
        col.last_payment = now;
        col.rebalance_volume = new_rebalance_volume;
        col.rebalance_count = new_rebalance_count;
        col.clean_volume = new_clean_volume;
        col.clean_count = new_clean_count;
    });
}

ACTION trail::unregworker(name worker_name) {
    //open workers table, get worker
    workers_table workers(get_self(), get_self().value);
    auto& wrk = workers.get(worker_name.value, "worker not found");

    //authenticate
    require_auth(wrk.worker_name);

    //validate
    // no validations necessary

    //NOTE: worker forfeits all current claim to rewards upon unregistration

    //erase worker
    workers.erase(wrk);
}

ACTION trail::claimpayment(name worker_name, symbol registry_symbol) {
    // open workers table, get worker
    workers_table workers(get_self(), get_self().value);
    auto& wrk = workers.get(worker_name.value);

    //authenticate
    require_auth(wrk.worker_name);

    //initialize
    asset total_tlos_payout = asset(0, TLOS_SYM);
    auto now = time_point_sec(current_time_point()).sec_since_epoch();

    //validate
    check(wrk.last_payment.sec_since_epoch() + 86400 > now, "can only claim payment once every 24 hours");
    check(wrk.standing == name("good"), "must be in good standing to claim payment");

    //diminish rate: 1% of payout, per day past 1 week of no claims
    double reduced_by = ((now - (wrk.last_payment.sec_since_epoch() + 604800)) / 86400) / 100;

    //reset worker data
    workers.modify(wrk, same_payer, [&](auto& col) {
        col.last_payment = time_point_sec(current_time_point());
        col.rebalance_volume[registry_symbol] = asset(0, registry_symbol);
        col.rebalance_count[registry_symbol] = uint16_t(0);
    });

    //TODO: update leaderboard
}

ACTION trail::rebalance(name voter, symbol registry_symbol, optional<uint16_t> count) {
    //open accounts table, get account
    accounts_table accounts(get_self(), voter.value);
    auto& acct = accounts.get(registry_symbol.code().raw(), "account not found");

    //open votes table, get bysymbol sec index
    votes_table votes(get_self(), voter.value);
    auto votes_bysymbol = votes.get_index<name("bysymbol")>();
    auto vote_sym_itr = votes_bysymbol.lower_bound(registry_symbol.code().raw());

    //validate
    

    //TODO: start at receipt first to expire?
    //TODO: if ballot_name supplied, start at ballot_name instead of lower bound

    //initialize
    auto now = time_point_sec(current_time_point());
    uint16_t remaining;
    uint16_t rebalanced = 0;
    asset rebalanced_volume = asset(0, registry_symbol);
    asset vote_weight = acct.balance;

    //TODO: change vote weight source if usestake

    (count) ? remaining = *count : remaining = 1;

    while (remaining > 0 && vote_sym_itr != votes_bysymbol.end()) {
        //intitialize
        auto curr_vote = *vote_sym_itr;

        //skip if expired or not correct symbol
        if (now > curr_vote.expiration || registry_symbol != curr_vote.registry_symbol) {
            vote_sym_itr++; 
            continue;
        }

        //open ballots table, get ballot
        ballots_table ballots(get_self(), get_self().value);
        auto& bal = ballots.get(curr_vote.ballot_name.value, "ballot not found");

        //intialize
        map<name, asset> new_bal_options = bal.options;
        vector<name> selected_options;
        asset vol = asset(0, registry_symbol);

        //rollback current voted options
        //TODO: rollback in order of weight so ranked ballots aren't corrupted when rebalanced
        for (auto i = curr_vote.options_voted.begin(); i == curr_vote.options_voted.end(); i++) {
            new_bal_options[i->first] -= i->second;
            selected_options.push_back(i->first);
        }

        //recalculate voted options
        map<name, asset> new_options_voted = calc_vote_mapping(registry_symbol, bal.voting_method, 
            selected_options, vote_weight);

        //apply new voted options to ballot
        //TODO: reapply votes in order of weight to preserve ranked votes
        for (auto i = new_options_voted.begin(); i == new_options_voted.end(); i++) {
            vol += curr_vote.options_voted[i->first];
            new_bal_options[i->first] += i->second;
        }

        //update voted options
        votes_bysymbol.modify(vote_sym_itr, same_payer, [&](auto& col) {
            col.options_voted = new_options_voted;
        });

        //update ballot
        ballots.modify(bal, same_payer, [&](auto& col) {
            col.options = new_bal_options;
        });

        //update counters
        remaining--;
        rebalanced++;
        rebalanced_volume += vol;

        //iterate vote_sym_itr
        vote_sym_itr++;
    }

    //TODO: update rebalanced volume on worker data (if not a worker then skip)

    //TODO: update rebalanced volume on registry

}

ACTION trail::cleanupvote(name voter, optional<uint16_t> count) {
    //sort votes by expiration, lowest first
    votes_table votes(get_self(), voter.value);
    auto votes_byexp = votes.get_index<name("byexp")>(); 
    auto byexp_itr = votes_byexp.begin();

    //initialize
    uint16_t cleaned = 0;
    auto now = time_point_sec(current_time_point());
    uint16_t to_clean = 1;

    if (count) {
        to_clean = *count;
    }

    //cleans expired votes until count reaches 0 or end of table, skips active votes
    while (to_clean > 0 && byexp_itr != votes_byexp.end()) {
        //check if vote has expired
        if (byexp_itr->expiration < now) { //expired
            byexp_itr = votes_byexp.erase(byexp_itr); //returns next iterator
            to_clean--;
            cleaned++;

            //TODO: update cleaned count and volume on ballot

        } else { //active
            byexp_itr++;
        }
    }

    //TODO: update worker data
    
}

//======================== committee actions ========================

ACTION trail::regcommittee(name committee_name, string committee_title,
    symbol registry_symbol, vector<name> initial_seats, name registree) {
    //authenticate
    require_auth(registree);

    //open committees table, search for committee
    committees_table committees(get_self(), registry_symbol.code().raw());
    auto cmt = committees.find(committee_name.value);

    //open accounts table, get account
    accounts_table accounts(get_self(), registree.value);
    auto& acct = accounts.get(registry_symbol.code().raw(), "account not found");

    //initialize
    map<name, name> new_seats;

    //validate
    check(cmt == committees.end(), "committtee already exists");
    check(committee_title.size() <= 256, "committee title has more than 256 bytes");

    for (name n : initial_seats) {
        check(new_seats.find(n) == new_seats.end(), "seat names must be unique");
        new_seats[n] = name(0);
    }

    //create committee
    committees.emplace(registree, [&](auto& col) {
        col.committee_title = committee_title;
        col.committee_name = committee_name;
        col.registry_symbol = registry_symbol;
        col.seats = new_seats;
        col.updater_acct = registree;
        col.updater_auth = name("active");
    });
}

ACTION trail::addseat(name committee_name, symbol registry_symbol, name new_seat_name) {
    //open committees table, get committee
    committees_table committees(get_self(), registry_symbol.code().raw());
    auto& cmt = committees.get(committee_name.value, "committee not found");
    
    //authenticate
    require_auth(permission_level{cmt.updater_acct, cmt.updater_auth});

    //validate
    check(cmt.seats.find(new_seat_name) == cmt.seats.end(), "seat already exists");

    //add seat to committee
    committees.modify(cmt, same_payer, [&](auto& col) {
        col.seats[new_seat_name] = name(0);
    });
}

ACTION trail::removeseat(name committee_name, symbol registry_symbol, name seat_name) {
    //open committees table, get committee
    committees_table committees(get_self(), registry_symbol.code().raw());
    auto& cmt = committees.get(committee_name.value, "committee not found");
    
    //authenticate
    require_auth(permission_level{cmt.updater_acct, cmt.updater_auth});

    //validate
    check(cmt.seats.find(seat_name) != cmt.seats.end(), "seat name not found");

    //remove seat from committee
    committees.modify(cmt, same_payer, [&](auto& col) {
        col.seats.erase(seat_name);
    });
}

ACTION trail::assignseat(name committee_name, symbol registry_symbol, name seat_name, name seat_holder, string memo) {
    //open committees table, get committee
    committees_table committees(get_self(), registry_symbol.code().raw());
    auto& cmt = committees.get(committee_name.value, "committee not found");
    
    //authenticate
    require_auth(permission_level{cmt.updater_acct, cmt.updater_auth});

    //validate
    check(cmt.seats.find(seat_name) != cmt.seats.end(), "seat name not found");

    //assign seat holder to seat on committee
    committees.modify(cmt, same_payer, [&](auto& col) {
        col.seats[seat_name] = seat_holder;
    });
}

ACTION trail::setupdater(name committee_name, symbol registry_symbol, name updater_account, name updater_auth) {
    //open committees table, get committee
    committees_table committees(get_self(), registry_symbol.code().raw());
    auto& cmt = committees.get(committee_name.value, "committee not found");
    
    //authenticate
    require_auth(permission_level{cmt.updater_acct, cmt.updater_auth});

    //set new committee updater account and updater auth
    committees.modify(cmt, same_payer, [&](auto& col) {
        col.updater_acct = updater_account;
        col.updater_auth = updater_auth;
    });
}

ACTION trail::delcommittee(name committee_name, symbol registry_symbol, string memo) {
    //open committees table, get committee
    committees_table committees(get_self(), registry_symbol.code().raw());
    auto& cmt = committees.get(committee_name.value, "committee not found");
    
    //authenticate
    require_auth(permission_level{cmt.updater_acct, cmt.updater_auth});

    //erase committee
    committees.erase(cmt);
}

//========== notification methods ==========

void trail::catch_delegatebw(name from, name receiver, asset stake_net_quantity, asset stake_cpu_quantity, bool transfer) {
    //validate (softfail)

    //open accounts table, search for account
    accounts_table accounts(get_self(), from.value);
    auto acct = accounts.find(VOTE_SYM.code().raw());

    //initialize
    asset total_staked = stake_net_quantity + stake_cpu_quantity;

    //add to vote stake
    if (acct != accounts.end()) { //account exists
        add_stake(from, asset(total_staked.amount, VOTE_SYM));
        
        //open registries table, search for registry
        registries_table registries(get_self(), get_self().value);
        auto reg = registries.find(VOTE_SYM.code().raw());

        //check if registry exists (should always be true)
        if (reg != registries.end()) {
            registries.modify(*reg, same_payer, [&](auto& col) {
                col.supply += asset(total_staked.amount, VOTE_SYM);
            });
        }

        //TODO: check that staked VOTE == staked TLOS
    }
}

void trail::catch_undelegatebw(name from, name receiver, asset unstake_net_quantity, asset unstake_cpu_quantity) {
    //authenticate

    //open accounts table, search for account
    accounts_table accounts(get_self(), from.value);
    auto acct = accounts.find(VOTE_SYM.code().raw());

    //initialize
    asset total_unstaked = unstake_net_quantity + unstake_cpu_quantity;

    //subtract from VOTE stake
    //TODO: overflow into balance if necessary (not applicable for VOTE)
    if (acct != accounts.end()) { //account exists
        sub_stake(from, total_unstaked);

        //open registries table, search for registry
        registries_table registries(get_self(), get_self().value);
        auto reg = registries.find(VOTE_SYM.code().raw());

        //check if registry exists (should always be true)
        if (reg != registries.end()) {
            registries.modify(*reg, same_payer, [&](auto& col) {
                col.supply -= asset(total_unstaked.amount, VOTE_SYM);
            });
        }
    }
}

void trail::catch_transfer(name from, name to, asset quantity, string memo) {
    //get initial receiver contract
    name rec = get_first_receiver();

    //validate
    if (rec == name("eosio.token") && from != get_self()) {
        //allows transfers to trail without triggering reaction
        if (memo == std::string("skip")) {
            return;
        } else if (memo == "deposit") {
            //deposit into account
            accounts_table accounts(get_self(), from.value);
            auto acct = accounts.find(TLOS_SYM.code().raw());

            if (acct == accounts.end()) { //no account
                accounts.emplace(get_self(), [&](auto& col) {
                    col.balance = quantity;
                    col.staked = asset(0, TLOS_SYM);
                });
            } else {
                accounts.modify(acct, same_payer, [&](auto& col) {
                    col.balance += quantity;
                });
            }
        }
    }
}

//========== utility methods ==========

void trail::add_balance(name voter, asset quantity) {
    //open accounts table, get account
    accounts_table to_accts(get_self(), voter.value);
    auto& to_acct = to_accts.get(quantity.symbol.code().raw(), "add_balance: account not found");

    //add quantity to liquid balance
    to_accts.modify(to_acct, same_payer, [&](auto& col) {
        col.balance += quantity;
    });
}

void trail::sub_balance(name voter, asset quantity) {
    //open accounts table, get account
    accounts_table from_accts(get_self(), voter.value);
    auto& from_acct = from_accts.get(quantity.symbol.code().raw(), "sub_balance: account not found");

    //subtract quantity from balance
    from_accts.modify(from_acct, same_payer, [&](auto& col) {
        col.balance -= quantity;
    });
}

void trail::add_stake(name voter, asset quantity) {
    //open accounts table, get account
    accounts_table to_accts(get_self(), voter.value);
    auto& to_acct = to_accts.get(quantity.symbol.code().raw(), "add_stake: account not found");

    //add quantity to stake
    to_accts.modify(to_acct, same_payer, [&](auto& col) {
        col.staked += quantity;
    });
}

void trail::sub_stake(name voter, asset quantity) {
    //open accounts table, get account
    accounts_table from_accts(get_self(), voter.value);
    auto& from_acct = from_accts.get(quantity.symbol.code().raw(), "sub_stake: account not found");

    //subtract quantity from stake
    from_accts.modify(from_acct, same_payer, [&](auto& col) {
        col.staked -= quantity;
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
        default:
            return false;
    }
}

void trail::add_rebalance_work(name worker_name, symbol registry_symbol, asset volume, uint16_t count) {
    //open workers table, search for worker
    workers_table workers(get_self(), get_self().value);
    auto wrk = workers.find(worker_name.value);

    if (wrk != workers.end()) { //worker exists
        workers.modify(wrk, same_payer, [&](auto& col) {
            col.rebalance_volume[registry_symbol] += volume;
            col.rebalance_count[registry_symbol] += count;
        });
    } else { //worker not found
        return;
    }
}

void trail::add_clean_work(name worker_name, name ballot_name, asset volume, uint16_t count) {
    //open workers table, search for worker
    workers_table workers(get_self(), get_self().value);
    auto wrk = workers.find(worker_name.value);

    if (wrk != workers.end()) { //worker exists
        workers.modify(wrk, same_payer, [&](auto& col) {
            col.clean_volume[ballot_name] += volume;
            col.clean_count[ballot_name] += count;
        });
    } else { //worker not found
        return;
    }
}

void trail::require_fee(name account_name, asset fee) {
    //open accounts table, get TLOS balance
    accounts_table tlos_accounts(get_self(), account_name.value);
    auto& tlos_acct = tlos_accounts.get(TLOS_SYM.code().raw(), "TLOS balance not found");

    //validate
    check(tlos_acct.balance >= fee, "insufficient funds to cover fee");

    //charge fee
    tlos_accounts.modify(tlos_acct, same_payer, [&](auto& col) {
        col.balance -= fee;
    });
}

map<name, asset> trail::calc_vote_mapping(symbol registry_symbol, name voting_method, 
    vector<name> selections,  asset raw_vote_weight) {
    //initialize
    map<name, asset> vote_mapping;
    int64_t effective_amount;
    int64_t vote_amount_per;
    int8_t pos = 1;

    switch (voting_method.value) {
        case (name("1acct1vote").value):
            //TODO: refactor so vote is always 1 whole token
            effective_amount = 10000;
            break;
        case (name("1tokennvote").value):
            effective_amount = raw_vote_weight.amount;
            break;
        case (name("1token1vote").value):
            effective_amount = raw_vote_weight.amount / selections.size();
            break;
        case (name("1tsquare1v").value):
            vote_amount_per = raw_vote_weight.amount / selections.size();
            effective_amount = vote_amount_per * vote_amount_per;
            //TODO: final squaring of each option required at end of ballot
            break;
        case (name("quadratic").value):
            effective_amount = sqrtl(raw_vote_weight.amount);
            break;
        case (name("ranked").value):
            for (name n : selections) {
                effective_amount = raw_vote_weight.amount / pos;
                vote_mapping[n] = asset(effective_amount, registry_symbol);
                pos++;
            }
            return vote_mapping;
        default:
            check(false, "calc_vote_mapping: invalid voting method");
    }

    for (name n : selections) {
        vote_mapping[n] = asset(effective_amount, registry_symbol);
    }

    return vote_mapping;
}
