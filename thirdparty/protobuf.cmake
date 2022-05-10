include(FetchContent)
set(FETCHCONTENT_QUIET ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(Git REQUIRED)

SET(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
SET(protobuf_BUILD_CONFORMANCE OFF CACHE BOOL "" FORCE)
SET(protobuf_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(protobuf_BUILD_EXPORT OFF CACHE BOOL "" FORCE)
SET(protobuf_WITH_ZLIB OFF CACHE BOOL "" FORCE)
FetchContent_Declare(
        protobuf
        GIT_REPOSITORY "https://github.com/protocolbuffers/protobuf.git"
        GIT_TAG v3.15.6
        SOURCE_SUBDIR cmake
)
FetchContent_MakeAvailable(protobuf)