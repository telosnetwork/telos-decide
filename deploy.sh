#! /bin/bash

account=""
url=""
contract=""
has_error=false

find_out=$(find ./contracts/ -type d -mindepth 1  -maxdepth 1 -exec basename {} \; | tr "\n" " ")
IFS=' ' && read -ra contracts_array <<< "$find_out"
len=${#contracts_array[@]} && options=""
for (( i=0; i<$len; i++)); do
    if [[ $i == $(($len-1)) ]]; then
        options+="${contracts_array[$i]}"
    else
        options+="${contracts_array[$i]}, "
    fi
done

usage_info() {
    echo "Usage: deploy.sh [-c] contract_name [-t] target_name [-a] account_name"
}

containsElement () {
  local e match="$1"
  shift
  for e; do [[ "$e" == "$match" ]] && return 0; done
  return 1
}

while getopts ":c:a:t:h" opt; do
  case ${opt} in
    c )
        contract=$OPTARG
      ;;
    t ) 
        if [[ $OPTARG == "mainnet" ]]; then
            url="https://telos.caleos.io/"
        elif [[ $OPTARG == "testnet" ]]; then
            url="https://testnet.telosusa.io/"
        elif [[ $OPTARG == "local" ]]; then
            url="http://127.0.0.1:8888/"
        else
            echo "Please provide a deployment target with [-t] {TARGET_OPTION}. Options: [ mainnet, testnet, local ]"
            exit 1
        fi
      ;;
    a )
        account=$OPTARG
      ;;
    h )
        usage_info
        exit 0
      ;;
  esac
done
shift $((OPTIND -1))

if [[ $url == "" ]]; then
    echo "[-t] flag must have an argument: -t {TARGET_OPTION}. Options: [ mainnet, testnet, local ]"
    has_error=true
fi

if [[ $contract == "" ]]; then
    base_out="[-c] flag must have an argument: -c {CONTRACT_NAME}. Options: [ "
    echo "${base_out}${options} ]"
    has_error=true
elif ! [[ " ${contracts_array[@]} " =~ " ${contract} " ]]; then
    echo "[-c] flag value must be a contract compile by this project [ ${options} ]"
    has_error=true
fi

if [[ $account == "" ]]; then
    echo "[-a] flag must have an argument: -a {VALID_ACCOUNT_NAME}"
    has_error=true
fi

if [[ $has_error == true ]]; then
    exit 1
fi

command="cleos -u $url set contract $account ./build/contracts/$contract/ $contract.wasm $contract.abi -p $account"
echo $command
eval $command