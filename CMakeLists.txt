cmake_minimum_required(VERSION 3.19)
project("learned-secondary-index" VERSION 1.0
    DESCRIPTION "A header only cmake/c++ library exposing <learned secondary index> implementation"
    HOMEPAGE_URL "https://github.com/learnedsystems/LearnedSecondaryIndex"
    LANGUAGES CXX)

# Declare library & directories to include. See
# http://mariobadr.com/creating-a-header-only-library-with-cmake.html for more
# info/install instructions
add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME} INTERFACE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>)

# ==== Dependencies ====
include(${PROJECT_SOURCE_DIR}/thirdparty/hashing.cmake)
include(${PROJECT_SOURCE_DIR}/thirdparty/learned_hashing.cmake)
include(${PROJECT_SOURCE_DIR}/thirdparty/absl.cmake)
include(${PROJECT_SOURCE_DIR}/thirdparty/boost.cmake)
include(${PROJECT_SOURCE_DIR}/thirdparty/protobuf.cmake)
target_link_libraries(${PROJECT_NAME} INTERFACE ${HASHING_LIBRARY} ${LEARNED_HASHING_LIBRARY}
    absl::core_headers absl::endian absl::span absl::strings
    Boost::dynamic_bitset
    absl::span
    libprotobuf-lite
    )

# Make IDE friendly
target_sources(${PROJECT_NAME} INTERFACE learned_secondary_index.hpp include/)

# Benchmark and test code
get_directory_property(hasParent PARENT_DIRECTORY)
if (NOT hasParent)
  add_subdirectory(src)
endif ()
