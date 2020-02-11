#include <decide.hpp>

using namespace decidespace;

ACTION decide::newtreasury(name manager, asset max_supply, name access) {
    
    //authenticate
    require_auth(manager);

    //open treasuries table, search for treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto trs = treasuries.find(max_supply.symbol.code().raw());

    //open configs singleton, get configs
    config_singleton configs(get_self(), get_self().value);
    check(configs.exists(), "telos.decide::init must be called before treasuries can be emplaced");
    auto conf = configs.get();

    //validate
    check(trs == treasuries.end(), "treasury already exists");
    check(max_supply.amount > 0, "max supply must be greater than 0");
    check(max_supply.symbol.is_valid(), "invalid symbol name");
    check(max_supply.is_valid(), "invalid max supply");
    check(valid_access_method(access), "invalid access method");

    //check reserved symbols
    if (manager != name("eosio")) {
        check(max_supply.symbol.code().raw() != TLOS_SYM.code().raw(), "TLOS symbol is reserved");
        check(max_supply.symbol.code().raw() != VOTE_SYM.code().raw(), "VOTE symbol is reserved");
    }

    //charge treasury fee
    require_fee(manager, conf.fees.at(name("treasury")));

    //set up initial settings
    map<name, bool> initial_settings;
    initial_settings[name("transferable")] = false;
    initial_settings[name("burnable")] = false;
    initial_settings[name("reclaimable")] = false;
    initial_settings[name("stakeable")] = false;
    initial_settings[name("unstakeable")] = false;
    initial_settings[name("maxmutable")] = false;

    //emplace new token treasury, RAM paid by manager
    treasuries.emplace(manager, [&](auto& col) {
        col.supply = asset(0, max_supply.symbol);
        col.max_supply = max_supply;
        col.access = access;
        col.manager = manager;
        col.title = "";
        col.description = "";
        col.icon = "";
        col.voters = uint32_t(0);
        col.delegates = uint32_t(0);
        col.committees = uint32_t(0);
        col.open_ballots = uint32_t(0);
        col.locked = false;
        col.unlock_acct = manager;
        col.unlock_auth = active_permission;
        col.settings = initial_settings;
    });

    //open payrolls table, find worker payroll
    payrolls_table payrolls(get_self(), max_supply.symbol.code().raw());
    auto pr = payrolls.find(name("workers").value);

    //open labor buckets table, find workers bucket
    laborbuckets_table laborbuckets(get_self(), max_supply.symbol.code().raw());
    auto lb = laborbuckets.find(name("workers").value);

    //validate
    check(pr == payrolls.end(), "workers payroll already exists");
    check(lb == laborbuckets.end(), "workers bucket already exists");

    //emplace worker payroll
    payrolls.emplace(manager, [&](auto& col) {
        col.payroll_name = name("workers");
        col.payroll_funds = asset(0, TLOS_SYM);
        col.period_length = 604800; //1 week in seconds
        col.per_period = asset(10, TLOS_SYM);
        col.last_claim_time = time_point_sec(current_time_point());
        col.claimable_pay = asset(0, TLOS_SYM);
        col.payee = name("workers");
    });

    //intialize
    map<name, asset> initial_claimable_volume;
    map<name, uint32_t> initial_claimable_events;

    initial_claimable_volume[name("rebalvolume")] = asset(0, max_supply.symbol);
    initial_claimable_events[name("rebalcount")] = 0;
    initial_claimable_events[name("cleancount")] = 0;

    //emplace labor bucket
    laborbuckets.emplace(manager, [&](auto& col) {
        col.payroll_name = name("workers");
        col.claimable_volume = initial_claimable_volume;
        col.claimable_events = initial_claimable_events;
    });

}

ACTION decide::edittrsinfo(symbol treasury_symbol, string title, string description, string icon) {
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(trs.manager);

    //validate
    check(!trs.locked, "treasury is locked");

    //update title, description, and icon
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.title = title;
        col.description = description;
        col.icon = icon;
    });
}

ACTION decide::toggle(symbol treasury_symbol, name setting_name) {
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(trs.manager);

    //validate
    check(!trs.locked, "treasury is locked");
    auto set_itr = trs.settings.find(setting_name);
    check(set_itr != trs.settings.end(), "setting not found");

    //update setting
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.settings[setting_name] = !trs.settings.at(setting_name);
    });

}

ACTION decide::mint(name to, asset quantity, string memo) {
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(quantity.symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(trs.manager);

    //validate
    check(is_account(to), "to account doesn't exist");
    check(trs.supply + quantity <= trs.max_supply, "minting would breach max supply");
    check(quantity.amount > 0, "must mint a positive quantity");
    check(quantity.is_valid(), "invalid quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //update recipient liquid amount
    add_liquid(to, quantity);

    //update treasury supply
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.supply += quantity;
    });

    //notify to account
    require_recipient(to);

}

ACTION decide::transfer(name from, name to, asset quantity, string memo) {
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(quantity.symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(from);

    //validate
    check(is_account(to), "to account doesn't exist");
    check(trs.settings.at(name("transferable")), "token is not transferable");
    check(from != to, "cannot transfer tokens to yourself");
    check(quantity.amount > 0, "must transfer positive quantity");
    check(quantity.is_valid(), "invalid quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //subtract quantity from sender liquid amount
    sub_liquid(from, quantity);

    //add quantity to recipient liquid amount
    add_liquid(to, quantity);

    //notify from and to accounts
    require_recipient(from);
    require_recipient(to);

}

ACTION decide::burn(asset quantity, string memo) {
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(quantity.symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(trs.manager);

    //open voters table, get manager
    voters_table voters(get_self(), trs.manager.value);
    auto& mgr = voters.get(quantity.symbol.code().raw(), "manager voter not found");

    //validate
    check(trs.settings.at(name("burnable")), "token is not burnable");
    check(trs.supply - quantity >= asset(0, quantity.symbol), "cannot burn supply below zero");
    check(mgr.liquid >= quantity, "burning would overdraw balance");
    check(quantity.amount > 0, "must burn a positive quantity");
    check(quantity.is_valid(), "invalid quantity");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //subtract quantity from manager liquid amount
    sub_liquid(trs.manager, quantity);

    //update treasury supply
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.supply -= quantity;
    });

    //notify manager account
    require_recipient(trs.manager);

}

ACTION decide::reclaim(name voter, asset quantity, string memo) {
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(quantity.symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(trs.manager);

    //validate
    check(trs.settings.at(name("reclaimable")), "token is not reclaimable");
    check(trs.manager != voter, "cannot reclaim tokens from yourself");
    check(is_account(voter), "voter account doesn't exist");
    check(quantity.is_valid(), "invalid amount");
    check(quantity.amount > 0, "must reclaim positive amount");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //sub quantity from liquid
    sub_liquid(voter, quantity);

    //add quantity to manager balance
    add_liquid(trs.manager, quantity);

}

ACTION decide::mutatemax(asset new_max_supply, string memo) {
    
    //get treasuries table, open treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(new_max_supply.symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(trs.manager);

    //validate
    check(trs.settings.at(name("maxmutable")), "max supply is not modifiable");
    check(new_max_supply.is_valid(), "invalid amount");
    check(new_max_supply.amount >= 0, "max supply cannot be below zero");
    check(new_max_supply >= trs.supply, "cannot lower max supply below current supply");
    check(memo.size() <= 256, "memo has more than 256 bytes");

    //update max supply
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.max_supply = new_max_supply;
    });

}

ACTION decide::setunlocker(symbol treasury_symbol, name new_unlock_acct, name new_unlock_auth) {
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(trs.manager);

    //validate
    check(!trs.locked, "treasury is locked");
    check(is_account(new_unlock_acct), "unlock account doesn't exist");

    //update unlock acct and auth
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.unlock_acct = new_unlock_acct;
        col.unlock_auth = new_unlock_auth;
    });

}

ACTION decide::lock(symbol treasury_symbol) {
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(trs.manager);
    check(!trs.locked, "treasury is already locked");

    //update lock
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.locked = true;
    });

}

ACTION decide::unlock(symbol treasury_symbol) {
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(permission_level{trs.unlock_acct, trs.unlock_auth});

    //validate
    check(trs.locked, "treasury is already unlocked");

    //update lock
    treasuries.modify(trs, same_payer, [&](auto& col) {
        col.locked = false;
    });

}

//======================== payroll actions ========================

ACTION decide::addfunds(name from, symbol treasury_symbol, asset quantity) {

    //authenticate
    require_auth(from);
    
    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //validate
    check(quantity.symbol == TLOS_SYM, "only TLOS allowed in payrolls");

    //open payrolls table, get payroll
    payrolls_table payrolls(get_self(), treasury_symbol.code().raw());
    auto& pr = payrolls.get(name("workers").value, "payroll not found");

    //charge quantity to account
    require_fee(from, quantity);

    //debit quantity to payroll funds
    payrolls.modify(pr, same_payer, [&](auto& col) {
        col.payroll_funds += quantity;
    });

}

ACTION decide::editpayrate(symbol treasury_symbol, uint32_t period_length, asset per_period) {
    
    //open payrolls table, get payroll
    payrolls_table payrolls(get_self(), treasury_symbol.code().raw());
    auto& pr = payrolls.get(name("workers").value, "payroll not found");

    //open treasuries table, get treasury
    treasuries_table treasuries(get_self(), get_self().value);
    auto& trs = treasuries.get(treasury_symbol.code().raw(), "treasury not found");

    //authenticate
    require_auth(trs.manager);

    //validate
    check(period_length > 0, "period length must be greater than 0");
    check(per_period.amount > 0, "per period pay must be greater than 0");
    check(per_period.symbol == TLOS_SYM, "only TLOS allowed in payrolls");

    //update pay rate
    payrolls.modify(pr, same_payer, [&](auto& col) {
        col.period_length = period_length;
        col.per_period = per_period;
    });

}