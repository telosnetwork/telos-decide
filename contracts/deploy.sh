#! /bin/bash

if [[ "$1" == "trail" ]]; then
    contract=trail
    account=trailservice
else
    echo "need contract"
    exit 0
fi

if [[ "$2" == "production" ]]; then
    url=http://api.tlos.goodblock.io
elif [[ "$2" == "test" ]]; then
    url=https://api-test.tlos.goodblock.io/
elif [[ "$2" == "local" ]]; then
    url=http://127.0.0.1:8888
else
    echo "need stage"
    exit 0
fi

#eos v1.7.0
cleos -u $url set contract $account ./build/$contract/ $contract.wasm $contract.abi -p $account