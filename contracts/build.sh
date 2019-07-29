#! /bin/bash

if [[ "$1" == "trail" ]]; then
    contract=trail
else
    echo "need contract"
    exit 0
fi

# -contract=<string>       - Contract name
# -o=<string>              - Write output to <file>
# -abigen                  - Generate ABI
# -I=<string>              - Add directory to include search path
# -L=<string>              - Add directory to library search path
# -R=<string>              - Add a resource path for inclusion

#eosio.cdt v1.6.1
eosio-cpp -I="./$contract/include/" -R="./$contract/resources" -o="./build/$contract/$contract.wasm" -contract="trail" -abigen ./$contract/src/$contract.cpp