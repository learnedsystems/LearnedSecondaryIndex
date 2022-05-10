#!/bin/bash

# Setup script
source .env
set -e
cd "$(dirname "$0")"

# Parse arguments
TARGET=${1:-"lsi_benchmarks"}
BUILD_TYPE=${2:-"RELEASE"}
BUILD_DIR="cmake-build-$(echo "${BUILD_TYPE}" | awk '{print tolower($0)}')"

# Build the target
./build.sh ${TARGET} ${BUILD_TYPE}

# Execute the target
${BUILD_DIR}/src/${TARGET} --benchmark_out=benchmark_results.json --benchmark_out_format=json
