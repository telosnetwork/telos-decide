#! /bin/bash

if [[ "$1" == "trail" ]]; then
    contract=trail
    account=trailservice
elif [[ "$1" == "example" ]]; then
    contract=example
    account=testaccounta
else
    echo "Please provide a contract name."
    exit 1
fi

if [[ "$2" == "production" ]]; then
    url=http://api.tlos.goodblock.io
elif [[ "$2" == "testnet" ]]; then
    url=https://api-test.tlos.goodblock.io/
elif [[ "$2" == "local" ]]; then
    url=http://127.0.0.1:8888
else
    echo "Please provide a deployment target"
    exit 1
fi

#eos v1.7.0
cleos -u $url set contract $account ./build/$contract/ $contract.wasm $contract.abi -p $account