#include <trail.hpp>

using namespace trailservice;

//======================== worker actions ========================

ACTION trail::forfeitwork(name worker_name, symbol treasury_symbol) {
    //open workers table, get worker
    labors_table labors(get_self(), treasury_symbol.code().raw());
    auto& lab = labors.get(worker_name.value, "labor not found");

    //open labor buckets table, get workers bucket
    laborbuckets_table laborbuckets(get_self(), treasury_symbol.code().raw());
    auto& bucket = laborbuckets.get(name("workers").value, "workers bucket not found");

    //authenticate
    require_auth(lab.worker_name);

    //initialize
    auto new_claimable_volume = bucket.claimable_volume;
    auto new_claimable_events = bucket.claimable_events;

    new_claimable_volume[name("rebalvolume")] -= lab.unclaimed_volume.at(name("rebalvolume"));
    new_claimable_events[name("rebalcount")] -= lab.unclaimed_events.at(name("rebalcount"));
    new_claimable_events[name("cleancount")] -= lab.unclaimed_events.at(name("cleancount"));

    //update labor bucket
    laborbuckets.modify(bucket, same_payer, [&](auto& col) {
        col.claimable_volume = new_claimable_volume;
        col.claimable_events = new_claimable_events;
    });

    //erase worker
    labors.erase(lab);
}

ACTION trail::claimpayment(name claimant, symbol treasury_symbol) {
    //open labors table, get labor
    labors_table labors(get_self(), treasury_symbol.code().raw());
    auto& lab = labors.get(claimant.value, "work not found");

    //open labor buckets table, get bucket
    laborbuckets_table laborbuckets(get_self(), treasury_symbol.code().raw());
    auto& bucket = laborbuckets.get(name("workers").value, "workers bucket not found");

    //open payrolls table, get worker payroll
    payrolls_table payrolls(get_self(), treasury_symbol.code().raw());
    auto& pr = payrolls.get(name("workers").value, "workers payroll not found");

    //authenticate
    require_auth(lab.worker_name);

    //initialize
    uint32_t now = time_point_sec(current_time_point()).sec_since_epoch();
    asset new_claimable_pay = pr.claimable_pay;
    asset additional_pay = asset(0, pr.payroll_funds.symbol);
    auto new_claimable_volume = bucket.claimable_volume;
    auto new_claimable_events = bucket.claimable_events;

    //validate
    check(lab.start_time.sec_since_epoch() + 86400 > now, "labor must mature for 1 day before claiming");
    check(pr.payee == name("workers"), "payroll not for workers");

    //advance payroll if needed
    if (pr.last_claim_time.sec_since_epoch() + pr.period_length < now) {
        uint32_t new_periods = (pr.last_claim_time.sec_since_epoch() - now) / pr.period_length;
        additional_pay = new_periods * pr.per_period;
        new_claimable_pay += additional_pay;

        //ensure additional pay doesn't overdraw funds
        if (pr.payroll_funds <= additional_pay) {
            additional_pay = pr.payroll_funds;
        }
    }

    //diminish rate: 1% of payout per day without claiming
    double reduced_by = ( (now - lab.start_time.sec_since_epoch() / 86400) - 1 ) / 100;

    //calculate worker payout
    asset payout = asset(0, pr.payroll_funds.symbol);
    asset trail_payout = asset(0, TRAIL_SYM);

    double vol_share = double(lab.unclaimed_volume.at(name("rebalvolume")).amount) / 
        double(bucket.claimable_volume.at(name("rebalvolume")).amount);

    double count_share = double(lab.unclaimed_events.at(name("rebalcount"))) / 
        double(bucket.claimable_events.at(name("rebalcount")));

    double clean_share = double(lab.unclaimed_events.at(name("cleancount"))) / 
        double(bucket.claimable_events.at(name("cleancount")));

    double total_share = (vol_share + count_share + clean_share) / double(3.0);
    payout = asset(int64_t(new_claimable_pay.amount * total_share), pr.payroll_funds.symbol);
    
    //TODO: reduce payout by percentage
    // payout.amount = uint64_t(double(payout.amount) * reduced_by);

    //TODO: calculate TRAIL payout

    if (payout > new_claimable_pay) {
        payout = new_claimable_pay;
    }

    //validate
    check(new_claimable_pay.amount > 0, "payroll is empty");

    new_claimable_pay -= payout;

    new_claimable_volume[name("rebalvolume")] -= lab.unclaimed_volume.at(name("rebalvolume"));
    new_claimable_events[name("rebalcount")] -= lab.unclaimed_events.at(name("rebalcount"));
    new_claimable_events[name("cleancount")] -= lab.unclaimed_events.at(name("cleancount"));

    //update labor bucket
    laborbuckets.modify(bucket, same_payer, [&](auto& col) {
        col.claimable_volume = new_claimable_volume;
        col.claimable_events = new_claimable_events;
    });

    //erase labor
    labors.erase(lab);

    //update payroll
    payrolls.modify(pr, same_payer, [&](auto& col) {
        col.payroll_funds = pr.payroll_funds - additional_pay;
        col.last_claim_time = time_point_sec(current_time_point());
        col.claimable_pay = new_claimable_pay;
    });

    //open accounts table, get account
    accounts_table accounts(get_self(), claimant.value);
    auto acct = accounts.find(pr.payroll_funds.symbol.code().raw());

    if (acct != accounts.end()) { //account found
        accounts.modify(acct, same_payer, [&](auto& col) {
            col.balance += payout;
        });
    } else {
        accounts.emplace(claimant, [&](auto& col) {
            col.balance = payout;
        });
    }

}

ACTION trail::rebalance(name voter, name ballot_name, optional<name> worker) {
    
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
    auto now = time_point_sec(current_time_point());
    asset raw_vote_weight = asset(0, bal.treasury_symbol);
    map<name, asset> new_bal_options = bal.options;
    vector<name> selections;
    name worker_name = name(0);

    //validate
    check(now < bal.end_time, "vote has already expired");

    if (bal.settings.at(name("votestake"))) { //use stake
        raw_vote_weight = vtr.staked;
    } else { //use liquid
        raw_vote_weight = vtr.liquid;
    }

    //rollback old vote
    for (auto i = v.weighted_votes.begin(); i != v.weighted_votes.end(); i++) {
        new_bal_options[i->first] -= i->second;

        //rebuild selections
        selections.push_back(i->first);
    }

    //validate
    check(raw_vote_weight != v.raw_votes, "vote is already balanced");
    check(selections.size() > 0, "cannot rebalance nonexistent votes");
    check(raw_vote_weight.amount > 0, "cannot vote with zero weight");

    //calculate new votes
    auto new_votes = calc_vote_weights(bal.treasury_symbol, bal.voting_method, selections, raw_vote_weight);
    int64_t weight_delta = abs(v.raw_votes.amount - raw_vote_weight.amount);

    //apply new votes to ballot
    for (auto i = new_votes.begin(); i != new_votes.end(); i++) {
        new_bal_options[i->first] += i->second;
    }

    //update ballot
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.options = new_bal_options;
        col.total_raw_weight += (raw_vote_weight - v.raw_votes);
    });

    //set worker info if applicable
    if (worker) {
        //initialize
        worker_name = *worker;

        //authenticate
        require_auth(worker_name);
    }

    //update vote
    votes.modify(v, same_payer, [&](auto& col) {
        col.raw_votes = raw_vote_weight;
        col.weighted_votes = new_votes;
        col.worker = worker_name;
        col.rebalances += 1;
        col.rebalance_volume = asset(weight_delta, bal.treasury_symbol);
    });

}

ACTION trail::cleanupvote(name voter, name ballot_name, optional<name> worker) {
    //open votes table, get vote
    votes_table votes(get_self(), ballot_name.value);
    auto& v = votes.get(voter.value, "vote not found");

    //open ballots table, get ballot
    ballots_table ballots(get_self(), get_self().value);
    auto& bal = ballots.get(ballot_name.value, "ballot not found");

    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(bal.treasury_symbol.code().raw(), "treasury not found");

    //initialize
    auto now = time_point_sec(current_time_point());

    //validate
    check(bal.end_time < now, "vote hasn't expired");
    
    //update ballot
    ballots.modify(bal, same_payer, [&](auto& col) {
        col.cleaned_count += 1;
    });

    //log worker share if worker
    if (worker) {
        name worker_name = *worker;

        //authenticate
        require_auth(worker_name);
        
        //update cleanup worker
        log_cleanup_work(worker_name, bal.treasury_symbol, 1);
    }

    //log rebalance work from vote
    if (v.worker != name(0)) {
        log_rebalance_work(v.worker, bal.treasury_symbol, v.rebalance_volume, 1);
    }

    //erase expired vote
    votes.erase(v);
    
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