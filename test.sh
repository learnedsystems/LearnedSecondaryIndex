#!/bin/bash

# setup script
set -e
cd "$(dirname "$0")"

# build and run tests
./build.sh lsi_tests DEBUG
cmake-build-debug/src/lsi_tests $@

./build.sh lsi_tests RELEASE
cmake-build-release/src/lsi_tests $@
