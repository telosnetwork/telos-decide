#! /bin/bash

if [[ "$1" == "trail" ]]; then
    contract=trail
elif [[ "$1" == "example" ]]; then
    contract=example
else
    echo "Please provide a contract name."
    exit 1
fi

# -contract=<string>       - Contract name
# -o=<string>              - Write output to <file>
# -abigen                  - Generate ABI
# -I=<string>              - Add directory to include search path
# -L=<string>              - Add directory to library search path
# -R=<string>              - Add a resource path for inclusion

#eosio.cdt v1.6.1
eosio-cpp -I="./$contract/include/" -R="./$contract/resources" -o="./build/$contract/$contract.wasm" -contract="$contract" -abigen "./$contract/src/$contract.cpp"