#include <decide.hpp>

using namespace decidespace;

//======================== committee actions ========================

ACTION decide::regcommittee(name committee_name, string committee_title,

    symbol treasury_symbol, vector<name> initial_seats, name registree) {
    //authenticate
    require_auth(registree);

    //open committees table, search for committee
    committees_table committees(get_self(), treasury_symbol.code().raw());
    auto cmt = committees.find(committee_name.value);

    //open voters table, get voter
    voters_table voters(get_self(), registree.value);
    auto& vtr = voters.get(treasury_symbol.code().raw(), "voter not found");

    //open configs singleton, get config
    config_singleton configs(get_self(), get_self().value);
    auto conf = configs.get();

    //initialize
    map<name, name> new_seats;

    //validate
    check(cmt == committees.end(), "committtee already exists");
    check(committee_title.size() <= 256, "committee title has more than 256 bytes");

    //charge committee fee
    require_fee(registree, conf.fees.at(committee_n));

    for (name n : initial_seats) {
        check(new_seats.find(n) == new_seats.end(), "seat names must be unique");
        new_seats[n] = name(0);
    }

    //create committee
    committees.emplace(registree, [&](auto& col) {
        col.committee_title = committee_title;
        col.committee_name = committee_name;
        col.treasury_symbol = treasury_symbol;
        col.seats = new_seats;
        col.updater_acct = registree;
        col.updater_auth = active_permission;
    });

}

ACTION decide::addseat(name committee_name, symbol treasury_symbol, name new_seat_name) {

    //open committees table, get committee
    committees_table committees(get_self(), treasury_symbol.code().raw());
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

ACTION decide::removeseat(name committee_name, symbol treasury_symbol, name seat_name) {

    //open committees table, get committee
    committees_table committees(get_self(), treasury_symbol.code().raw());
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

ACTION decide::assignseat(name committee_name, symbol treasury_symbol, name seat_name, name seat_holder, string memo) {

    //open committees table, get committee
    committees_table committees(get_self(), treasury_symbol.code().raw());
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

ACTION decide::setupdater(name committee_name, symbol treasury_symbol, name updater_account, name updater_auth) {

    //open committees table, get committee
    committees_table committees(get_self(), treasury_symbol.code().raw());
    auto& cmt = committees.get(committee_name.value, "committee not found");
    
    //authenticate
    require_auth(permission_level{cmt.updater_acct, cmt.updater_auth});
    require_auth(permission_level{updater_account, updater_auth});

    //set new committee updater account and updater auth
    committees.modify(cmt, updater_account, [&](auto& col) {
        col.updater_acct = updater_account;
        col.updater_auth = updater_auth;
    });

}

ACTION decide::delcommittee(name committee_name, symbol treasury_symbol, string memo) {

    //open committees table, get committee
    committees_table committees(get_self(), treasury_symbol.code().raw());
    auto& cmt = committees.get(committee_name.value, "committee not found");
    
    //authenticate
    require_auth(permission_level{cmt.updater_acct, cmt.updater_auth});

    //erase committee
    committees.erase(cmt);

}