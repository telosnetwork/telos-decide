
#include <eosio/eosio.hpp>

//system defs
struct user_resources {
    name owner;
    asset net_weight;
    asset cpu_weight;
    int64_t ram_bytes = 0;

    uint64_t primary_key()const { return owner.value; }
    EOSLIB_SERIALIZE( user_resources, (owner)(net_weight)(cpu_weight)(ram_bytes) )
};

typedef multi_index<name("userres"), user_resources> user_resources_table;