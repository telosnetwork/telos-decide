// Example contract that listens for results from Trail and responds accordingly.
//
// @author Craig Branscom
// @contract example

#include <eosio/eosio.hpp>
#include <eosio/asset.hpp>
#include <eosio/action.hpp>

using namespace std;
using namespace eosio;

CONTRACT example : public contract {

public:

    example(name self, name code, datastream<const char*> ds);

    ~example();

    //======================== actions ========================

    // create desctription
    ACTION watchballot(name ballot_name, symbol registry_symbol, name committee_name, name seat_name);

    //======================== notification actions ========================

    [[eosio::on_notify("trailservice::bcastresults")]]
    void catch_bcast(name ballot_name, map<name, asset> final_results, uint32_t total_voters);

    //======================== tables ========================

    //table description
    //scope: self
    TABLE open_ballot {
        name ballot_name;
        symbol registry_symbol;
        name committee_name;
        name seat_name;

        uint64_t primary_key() const { return ballot_name.value; }
        EOSLIB_SERIALIZE(open_ballot, (ballot_name)(registry_symbol)(committee_name)(seat_name))
    };
    typedef multi_index<name("openballots"), open_ballot> openballots_table;

};