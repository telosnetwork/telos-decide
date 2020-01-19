#include <watcher.hpp>

watcher::watcher(name self, name code, datastream<const char*> ds) : contract(self, code, ds) { }

watcher::~watcher() { }

//======================== actions ========================

ACTION watcher::watchballot(name ballot_name, symbol treasury_symbol, name committee_name, name seat_name) {
    
    //open open_ballots table, search for open ballot
    openballots_table openballots(get_self(), get_self().value);
    auto ob = openballots.find(ballot_name.value);

    //validate
    check(ob == openballots.end(), "ballot is already being watched");

    //emplace open ballot
    openballots.emplace(get_self(), [&](auto& col) {
        col.ballot_name = ballot_name;
        col.treasury_symbol = treasury_symbol;
        col.committee_name = committee_name;
        col.seat_name = seat_name;
    });

}

void watcher::catch_broadcast(name ballot_name, map<name, asset> final_results, uint32_t total_voters) {
    
    //get initial receiver contract
    name rec = get_first_receiver();

    if (rec == name("telos.decide")) {
        
        //open open_ballots table, search for open ballot
        openballots_table openballots(get_self(), get_self().value);
        auto ob = openballots.find(ballot_name.value);

        if (ob != openballots.end()) {

            symbol treasury_symbol;
            name new_seat_holder;
            asset most_votes = asset(0, treasury_symbol);

            for (auto itr = final_results.begin(); itr != final_results.end(); itr++) {
                if (final_results[itr->first] > most_votes) {
                    new_seat_holder = itr->first;
                    most_votes = itr->second;
                    treasury_symbol = itr->second.symbol;
                }
            }
            
            //requires get_self()@eosio.code to be under get_self()@active
            action(permission_level{get_self(), name("active")}, name("telos.decide"), name("assignseat"), make_tuple(
                ob->committee_name, //committee_name
                ob->treasury_symbol, //treasury_symbol
                ob->seat_name, //seat_name
                new_seat_holder, //seat_holder
                std::string("auto-assigned from ballot results") //memo
            )).send();

        }
    }
}
