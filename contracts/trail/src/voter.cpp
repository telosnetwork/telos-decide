#include <trail.hpp>

//======================== voter actions ========================

ACTION trail::regvoter(name voter, symbol treasury_symbol, optional<name> referrer) {
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //open voters table, search for voter
    voters_table voters(get_self(), voter.value);
    auto vtr_itr = voters.find(treasury_symbol.code().raw());

    //validate
    check(is_account(voter), "voter account doesn't exist");
    check(vtr_itr == voters.end(), "voter already exists");
    check(treasury_symbol != TLOS_SYM, "cannot register as TLOS voter, use VOTE instead");
    check(treasury_symbol != TRAIL_SYM, "cannot register as TRAIL voter");

    //initialize
    name ram_payer = voter;

    if (referrer) {
        check(is_account(*referrer), "referring account must already exist");
    }

    //authenticate
    switch (trs.access.value) {
        case (name("public").value):
            if (referrer) {
                require_auth(*referrer);
                ram_payer = *referrer;
            } else {
                require_auth(voter);
            }
            break;
        case (name("private").value):
            if (referrer) {
                //authenticate
                require_auth(*referrer);
                //check referrer is treasury manager
                check(*referrer == trs.manager, "referrer must be treasury manager");
                //set referrer as ram payer
                ram_payer = *referrer;
            } else {
                require_auth(trs.manager);
            }
            break;
        case (name("invite").value):
            if (referrer) {
                //authenticate
                require_auth(*referrer);

                //TODO: check referrer is a voter

                //set referrer as ram payer
                ram_payer = *referrer;
            } else {
                require_auth(trs.manager);
            }
            break;
        case (name("membership").value):
            //inline sent from trailservice@membership
            require_auth(permission_level{get_self(), name("membership")});
            ram_payer = get_self();
            //TODO: membership payment features
            break;
        default:
            check(false, "invalid access method. contact treasury manager.");
    }

    //emplace new voter
    voters.emplace(ram_payer, [&](auto& col) {
        col.liquid = asset(0, treasury_symbol);
        col.staked = asset(0, treasury_symbol);
        col.staked_time = time_point_sec(current_time_point());
        col.delegated = asset(0, treasury_symbol);
        col.delegated_to = name(0);
        col.delegation_time = time_point_sec(current_time_point());
    });

    //update treasury
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.voters += 1;
    });
}

ACTION trail::unregvoter(name voter, symbol treasury_symbol) {
    
    //authenticate
    require_auth(voter);

    //open voters table, get account
    voters_table voters(get_self(), voter.value);
    auto& vtr = voters.get(treasury_symbol.code().raw(), "voter not found");

    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //validate
    check(vtr.liquid == asset(0, treasury_symbol), "cannot unregister unless liquid is zero");
    check(vtr.staked == asset(0, treasury_symbol), "cannot unregister unless staked is zero");

    //TODO: let voter unregister anyway by sending liquid and staked amount to manager?

    //TODO: require voter to cleanup/unvote all existing vote receipts first?

    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.voters -= 1;
    });

    //erase account
    voters.erase(vtr);

}

ACTION trail::castvote(name voter, name ballot_name, vector<name> options) {
    
    //authenticate
    require_auth(voter);

    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //open voters table, get voter
    voters_table voters(get_self(), voter.value);
    auto& vtr = voters.get(bal.treasury_symbol.code().raw(), "voter not found");

    //initialize
    auto now = time_point_sec(current_time_point());
    asset raw_vote_weight = asset(0, bal.treasury_symbol);
    uint32_t new_voter = 1;
    map<name, asset> temp_bal_options = bal.options;

    if (bal.settings.at(name("votestake"))) { //use stake
        raw_vote_weight = vtr.staked;
    } else { //use liquid
        raw_vote_weight = vtr.liquid;
    }

    asset raw_delta = raw_vote_weight;

    //validate
    check(bal.status == name("voting"), "ballot status is must be in voting mode to cast vote");
    check(now >= bal.begin_time && now <= bal.end_time, "vote must occur between ballot begin and end times");
    check(options.size() >= bal.min_options, "cannot vote for fewer than min options");
    check(options.size() <= bal.max_options, "cannot vote for more than max options");
    check(raw_vote_weight.amount > 0, "must vote with a positive amount");

    //skip vote tracking if light ballot
    if (bal.settings.at(name("lightballot"))) {
        return;
    }

    //open votes table, search for existing vote
    votes_table votes(get_self(), ballot_name.value);
    auto v_itr = votes.find(voter.value);

    //rollback if vote already exists
    if (v_itr != votes.end()) {
        
        //initialize
        auto v = *v_itr;
        raw_delta -= v.raw_votes;

        //validate
        check(bal.settings.at(name("revotable")), "ballot is not revotable");

        //rollback if weighted votes are not empty
        if (!v.weighted_votes.empty()) {
            
            //rollback old votes
            for (auto i = v.weighted_votes.begin(); i != v.weighted_votes.end(); i++) {
                temp_bal_options[i->first] -= i->second;
            }

            //update new voter
            new_voter = 0;
        }
    }

    //calculate new votes
    auto new_votes = calc_vote_weights(bal.treasury_symbol, bal.voting_method, options, raw_vote_weight);

    //apply new votes
    for (auto i = new_votes.begin(); i != new_votes.end(); i++) {
        
        //validate
        check(temp_bal_options.find(i->first) != temp_bal_options.end(), "option doesn't exist on ballot");

        //apply effective vote to ballot option
        temp_bal_options[i->first] += i->second;
    }

    //update ballot
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.options = temp_bal_options;
        col.total_voters += new_voter;
        col.total_raw_weight += raw_delta;
    });

    //update existing votes, or emplace votes if new
    if (new_voter == 1) {
        votes.emplace(voter, [&](auto& col) {
            col.voter = voter;
            col.is_delegate = false;
            col.raw_votes = raw_vote_weight;
            col.weighted_votes = new_votes;
            col.vote_time = time_point_sec(current_time_point());
            col.worker = name(0);
            col.rebalances = uint8_t(0);
            col.rebalance_volume = asset(0, bal.treasury_symbol);
        });
    } else {
        //update votes
        votes.modify(v_itr, same_payer, [&](auto& col) {
            col.raw_votes = raw_vote_weight;
            col.weighted_votes = new_votes;
        });
    }

}

ACTION trail::unvoteall(name voter, name ballot_name) {
    
    //authenticate
    require_auth(voter);

    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //open voters table, get voter
    voters_table voters(get_self(), voter.value);
    auto& vtr = voters.get(bal.treasury_symbol.code().raw(), "voter not found");

    //open votes table, get vote
    votes_table votes(get_self(), ballot_name.value);
    auto& v = votes.get(voter.value, "vote not found");

    //initialize
    map<name, asset> temp_bal_options = bal.options;
    auto now = time_point_sec(current_time_point());

    //validate
    check(bal.status == name("voting"), "ballot must be in voting mode to unvote");
    check(now >= bal.begin_time && now <= bal.end_time, "must unvote between begin and end time");
    check(!v.weighted_votes.empty(), "votes are already empty");

    //return if light ballot
    if (bal.settings.at(name("lightballot"))) {
        return;
    }

    //rollback old votes
    for (auto i = v.weighted_votes.begin(); i != v.weighted_votes.end(); i++) {
        temp_bal_options[i->first] -= i->second;
    }

    //update ballot
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.options = temp_bal_options;
        col.total_voters -= 1;
        col.total_raw_weight -= v.raw_votes;
    });

    //clear all votes in map (preserves rebalance count)
    votes.modify(v, same_payer, [&](auto& col) {
        col.raw_votes = asset(0, bal.treasury_symbol);
        col.weighted_votes.clear();
    });
    
}

ACTION trail::stake(name voter, asset quantity) {
    
    //authenticate
    require_auth(voter);

    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(quantity.symbol.code().raw(), "treasury not found");

    //validate
    check(trs.settings.at(name("stakeable")), "token is not stakeable");
    check(is_account(voter), "voter account doesn't exist");
    check(quantity.is_valid(), "invalid amount");
    check(quantity.amount > 0, "must stake positive amount");

    //subtract quantity from liquid
    sub_liquid(voter, quantity);

    //add quantity to staked
    add_stake(voter, quantity);

}

ACTION trail::unstake(name voter, asset quantity) {
    
    //authenticate
    require_auth(voter);

    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(quantity.symbol.code().raw(), "treasury not found");

    //validate
    check(trs.settings.at(name("unstakeable")), "token is not unstakeable");
    check(is_account(voter), "voter account doesn't exist");
    check(quantity.is_valid(), "invalid amount");
    check(quantity.amount > 0, "must unstake positive amount");

    //subtract quantity from stake
    sub_stake(voter, quantity);

    //add quantity to liquid
    add_liquid(voter, quantity);

}