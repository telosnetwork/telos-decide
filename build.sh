#! /bin/bash

RED='\033[0;31m'
NC='\033[0m'

printf "${RED}\t=========== Building Trail Service ===========${NC}\n\n"

CORES=`getconf _NPROCESSORS_ONLN`
mkdir -p build
pushd build &> /dev/null
cmake ../
make -j${CORES}
popd &> /dev/null