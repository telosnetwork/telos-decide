// Watcher contract that listens for results from Decide and responds accordingly.
//
// @author Craig Branscom
// @contract watcher

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>

using namespace std;
using namespace eosio;

CONTRACT watcher : public contract {

public:

    watcher(name self, name code, datastream<const char*> ds);

    ~watcher();

    //======================== actions ========================

    //start watching a ballot
    ACTION watchballot(name ballot_name, symbol treasury_symbol, name committee_name, name seat_name);

    //======================== notification actions ========================

    [[eosio::on_notify("telos.decide::broadcast")]]
    void catch_broadcast(name ballot_name, map<name, asset> final_results, uint32_t total_voters);

    //======================== tables ========================

    //watched ballots
    //scope: self
    TABLE open_ballot {
        name ballot_name;
        symbol treasury_symbol;
        name committee_name;
        name seat_name;

        uint64_t primary_key() const { return ballot_name.value; }
        EOSLIB_SERIALIZE(open_ballot, (ballot_name)(treasury_symbol)(committee_name)(seat_name))
    };
    typedef multi_index<name("openballots"), open_ballot> openballots_table;

};