cmake_minimum_required(VERSION 3.19)

include(CheckCXXCompilerFlag)
include(GoogleTest)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pthread")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -O0 -g -fsanitize=address,leak,undefined")
set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG")

# Enable march=native if available
check_cxx_compiler_flag(-march=native HAS_MARCH_NATIVE)
if (HAS_MARCH_NATIVE)
  target_compile_options(${PROJECT_NAME} INTERFACE -march=native)
endif ()

# ==== Non compatible compile flags ====
check_cxx_compiler_flag(-static-libsan HAS_LIBSAN)
if (HAS_LIBSAN)
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -static-libsan")
else ()
  set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -static-libasan")
endif ()

# ==== Competitors target ====
set(COMPETITORS_LIBRARY lsi_competitors)
add_library(${COMPETITORS_LIBRARY} INTERFACE)
target_sources(${COMPETITORS_LIBRARY} INTERFACE competitors.hpp)

include(${PROJECT_SOURCE_DIR}/thirdparty/tsl.cmake)
include(${PROJECT_SOURCE_DIR}/thirdparty/tlx.cmake)
add_custom_target(
    libfast64
    COMMAND cargo build --release
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/src/competitors/fast64/
)
add_dependencies(${COMPETITORS_LIBRARY} libfast64)
target_link_libraries(${COMPETITORS_LIBRARY}
    INTERFACE ${TLX}
    tsl::robin_map
    INTERFACE ${CMAKE_SOURCE_DIR}/src/competitors/fast64/target/release/libfast64.a -ldl
    )

# ==== Test target ====
set(TEST_TARGET lsi_tests)
add_executable(${TEST_TARGET} tests.cpp)

include(${PROJECT_SOURCE_DIR}/thirdparty/googletest.cmake)
target_link_libraries(${TEST_TARGET} PRIVATE ${PROJECT_NAME} ${COMPETITORS_LIBRARY} ${GOOGLETEST_LIBRARY})

# enable ctest support (i.e., test discovery)
gtest_discover_tests(${TEST_TARGET})

# ==== Benchmarks target ====
set(BENCHMARKS_TARGET lsi_benchmarks)
add_executable(${BENCHMARKS_TARGET} benchmarks.cpp)

include(${PROJECT_SOURCE_DIR}/thirdparty/googlebenchmark.cmake)
target_link_libraries(${BENCHMARKS_TARGET} PRIVATE ${PROJECT_NAME} ${COMPETITORS_LIBRARY} ${GOOGLEBENCHMARK_LIBRARY})
