#! /bin/bash

contract=""
did_init=false
RED='\033[0;31m'
NC='\033[0m'
CORES=`getconf _NPROCESSORS_ONLN`

usage_info() {
    echo "Usage: build.sh [-c] contract_name"
}

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

if ! [[ -d "./build" ]]; then
    printf "${RED}\tBuild folder not found, generating cmake files\n\n${NC}"
    printf "\t=========== Building Telos Decide ===========\n\n"
    mkdir -p build
    pushd build &> /dev/null
    cmake ../
    make -j${CORES}
    popd &> /dev/null
    did_init=true
fi

while getopts ":c:h" opt; do
  case ${opt} in
    c )
        contract=$OPTARG
      ;;
    h )
        usage_info
        exit 0
      ;;
  esac
done
shift $((OPTIND -1))

if [[ $contract != "" ]]; then
    if [[ " ${contracts_array[@]} " =~ " ${contract} " ]]; then
        printf "\t=========== Building ${contract} Contract ===========\n\n"
        pushd build/contracts/${contract} &> /dev/null
        make -j${CORES}
        popd &> /dev/null
    else
        echo "[-c] flag value must be the name of a contract compiled by this project. Options: [ ${options} ]"
        exit 1
    fi
elif [[ $did_init != true ]]; then
    pushd build &> /dev/null
    make -j${CORES}
    popd &> /dev/null
fi
