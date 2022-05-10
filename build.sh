#!/bin/bash

# Setup script
source .env
set -e
cd "$(dirname "$0")"

# Parse arguments
TARGET=${1:-"lsi_benchmarks"}
BUILD_TYPE=${2:-"RELEASE"}
BUILD_DIR="cmake-build-$(echo "${BUILD_TYPE}" | awk '{print tolower($0)}')/"

# Generate cmake project files
cmake \
  -D CMAKE_BUILD_TYPE=${BUILD_TYPE} \
  -D CMAKE_C_COMPILER=${C_COMPILER} \
  -D CMAKE_CXX_COMPILER=${CXX_COMPILER} \
  -B ${BUILD_DIR} \
  .

# Link compile_commands.json
ln -fs ${BUILD_DIR}compile_commands.json compile_commands.json

# Build tests code
cmake \
  --build ${BUILD_DIR} \
  --target ${TARGET} \
  -j
