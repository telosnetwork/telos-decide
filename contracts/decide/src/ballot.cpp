#include <decide.hpp>

using namespace decidespace;

//======================== ballot actions ========================

ACTION decide::newballot(name ballot_name, name category, name publisher,  
    symbol treasury_symbol, name voting_method, vector<name> initial_options) {
   
    //authenticate
    require_auth(publisher);

    //open configs singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //open voters table, get voter
    voters_table voters(get_self(), publisher.value);
    auto& vtr = voters.get(treasury_symbol.code().raw(), "voter not found");

    //open ballots table
    ballots_table ballots(get_self(), get_self().value);
    auto bal = ballots.find(ballot_name.value);

    //charge ballot listing fee to publisher
    require_fee(publisher, conf.fees.at(name("ballot")));

    //validate
    check(bal == ballots.end(), "ballot name already exists");
    check(valid_category(category), "invalid category");
    check(valid_voting_method(voting_method), "invalid voting method");

    //create initial options map, initial settings map
    map<name, asset> new_initial_options;
    map<name, bool> new_settings;

    //loop and assign initial options
    //NOTE: duplicates are OK, they will be consolidated into 1 key anyway
    for (name n : initial_options) {
        new_initial_options[n] = asset(0, treasury_symbol);
    }

    //intitial settings
    new_settings[name("lightballot")] = false;
    new_settings[name("revotable")] = true;
    new_settings[name("voteliquid")] = false;
    new_settings[name("votestake")] = trs.settings.at("stakeable"_n);
    //TODO: new_settings[name("allowdgate")] = false;

    //emplace new ballot
    ballots.emplace(publisher, [&](auto& col){
        col.ballot_name = ballot_name;
        col.category = category;
        col.publisher = publisher;
        col.status = name("setup");
        col.title = "";
        col.description = "";
        col.content = "";
        col.treasury_symbol = treasury_symbol;
        col.voting_method = voting_method;
        col.min_options = 1;
        col.max_options = 1;
        col.options = new_initial_options;
        col.total_voters = 0;
        col.total_delegates = 0;
        col.total_raw_weight = asset(0, treasury_symbol);
        col.cleaned_count = 0;
        col.settings = new_settings;
        col.begin_time = time_point_sec(0);
        col.end_time = time_point_sec(0);
    });

}

ACTION decide::editdetails(name ballot_name, string title, string description, string content) {
    
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
        col.content = content;
    });

}

ACTION decide::togglebal(name ballot_name, name setting_name) {
    
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

ACTION decide::editminmax(name ballot_name, uint8_t new_min_options, uint8_t new_max_options) {
    
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.status == name("setup"), "ballot must be in setup mode to edit max options");
    check(new_min_options > 0 && new_max_options > 0, "min and max options must be greater than zero");
    check(new_max_options >= new_min_options, "max must be greater than or equal to min");
    check(new_max_options <= bal.options.size(), "max options cannot be greater than number of options");

    //update ballot settings
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.min_options = new_min_options;
        col.max_options = new_max_options;
    });

}

ACTION decide::addoption(name ballot_name, name new_option_name) {
    
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.status == name("setup"), "ballot must be in setup mode to add options");
    check(bal.options.find(new_option_name) == bal.options.end(), "option is already in ballot");

    ballots.modify(bal, same_payer, [&](auto& col) {
        col.options[new_option_name] = asset(0, bal.treasury_symbol);
    });

}

ACTION decide::rmvoption(name ballot_name, name option_name) {
    
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

        if (col.min_options > col.options.size()) {
            col.min_options = col.options.size();
        }

        if (col.max_options > col.options.size()) {
            col.max_options = col.options.size();
        }
    });

}

ACTION decide::openvoting(name ballot_name, time_point_sec end_time) {
    
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

    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(bal.treasury_symbol.code().raw(), "treasury not found");

    //update open ballots on treasury
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.open_ballots += 1;
    });

    //validate
    check(bal.options.size() >= 2, "ballot must have at least 2 options");
    check(bal.status == name("setup"), "ballot must be in setup mode to ready");
    check(end_time.sec_since_epoch() > now.sec_since_epoch(), "end time must be in the future");
    check(end_time.sec_since_epoch() - now.sec_since_epoch() >= conf.times.at(name("minballength")), "ballot must be open for minimum ballot length");

    ballots.modify(bal, same_payer, [&](auto& col) {
        col.status = name("voting");
        col.begin_time = now;
        col.end_time = end_time;
    });

}

ACTION decide::cancelballot(name ballot_name, string memo) {
    
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.status == name("voting"), "ballot must be in voting mode to cancel");

    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(bal.treasury_symbol.code().raw(), "treasury not found");

    //update open ballots on treasury
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.open_ballots -= 1;
    });

    //update ballot status
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.status = name("cancelled");
    });

}

ACTION decide::deleteballot(name ballot_name) {
    
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
    check(now > bal.end_time + conf.times.at(name("balcooldown")), "cannot delete until 5 days past ballot's end time");
    check(bal.cleaned_count == bal.total_voters, "must clean all ballot votes before deleting");

    //erase ballot
    ballots.erase(bal);

}

ACTION decide::postresults(name ballot_name, map<name, asset> light_results, uint32_t total_voters) {
    
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //authenticate
    require_auth(bal.publisher);

    //validate
    check(bal.settings.at(name("lightballot")), "ballot is not a light ballot");
    check(bal.status == name("voting"), "ballot must be in voting mode to post results");
    check(bal.end_time < time_point_sec(current_time_point()), "must be past ballot end time to post");

    for (auto i = light_results.begin(); i != light_results.end(); i++) {
        check(i->second.symbol == bal.treasury_symbol, "result has incorrect symbol");
    }
    
    //apply results to ballot
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.options = light_results;
        col.total_voters = total_voters;
        col.cleaned_count = total_voters;
    });

}

ACTION decide::closevoting(name ballot_name, bool broadcast) {
    
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

    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(bal.treasury_symbol.code().raw(), "treasury not found");

    //update open ballots on treasury
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.open_ballots -= 1;
    });

    //perform 1tokensquare1v final sqrt()
    //NOTE: lightballots will already have sqrt() applied
    if (bal.voting_method == name("1tsquare1v") && !bal.settings.at(name("lightballot"))) {
        map<name, asset> squared_options = bal.options;

        //square root total votes on each option
        for (auto i = squared_options.begin(); i != squared_options.end(); i++) {
            squared_options[i->first] = asset(sqrtl(i->second.amount), bal.treasury_symbol);
        }

        //update vote counts
        ballots.modify(bal, same_payer, [&](auto& col) {
            col.options = squared_options;
        });
    }

    //if broadcast true, send broadcast inline to self
    if (broadcast) {
        broadcast_action broadcast_act(get_self(), { get_self(), active_permission });
        broadcast_act.send(ballot_name, bal.options, bal.total_voters);
    }

}

ACTION decide::broadcast(name ballot_name, map<name, asset> final_results, uint32_t total_voters) {
    //authenticate
    //TODO: require_auth(permission_level{get_self(), name("postresults")});
    require_auth(get_self());

    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value);

    //initialize
    auto now = time_point_sec(current_time_point());

    //validate
    check(now > bal.end_time, "ballot voting must be complete before broadcasting results");

    //notify ballot publisher (for external contract processing)
    require_recipient(bal.publisher);
}

ACTION decide::archive(name ballot_name, time_point_sec archived_until) {
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //open configs singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //open archivals table, search for archival
    archivals_table archivals(get_self(), get_self().value);
    auto arch = archivals.find(ballot_name.value);

    //authenticate
    require_auth(bal.publisher);

    //initialize
    auto now = time_point_sec(current_time_point()).sec_since_epoch();
    uint32_t days_to_archive = (archived_until.sec_since_epoch() - now) / uint32_t(86400) + 1;

    //validate
    check(arch == archivals.end(), "ballot is already archived");
    check(bal.status == name("closed"), "ballot must be closed to archive");
    check(archived_until.sec_since_epoch() > now, "archived until must be in the future");
    
    //calculate archive fee
    asset archival_fee = asset(conf.fees.at(name("archive")).amount * int64_t(days_to_archive), TLOS_SYM);

    //charge ballot publisher total fee
    require_fee(bal.publisher, archival_fee);

    //emplace in archived table
    archivals.emplace(bal.publisher, [&](auto& col) {
        col.ballot_name = ballot_name;
        col.archived_until = archived_until;
    });

    //change ballot status to archived
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.status = name("archived");
    });

}

ACTION decide::unarchive(name ballot_name, bool force) {
    
    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //open archivals table, get archival
    archivals_table archivals(get_self(), get_self().value);
    auto& arch = archivals.get(ballot_name.value, "archival not found");

    //initialize
    auto now = time_point_sec(current_time_point());

    //authenticate if force unarchive is true
    if (force) {
        //authenticate
        require_auth(bal.publisher);
    } else {
        //validate
        check(arch.archived_until < now, "ballot hasn't reached end of archival time");
    }

    //change ballot status to closed
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.status = name("closed");
    });

    //erase archival
    archivals.erase(arch);

}