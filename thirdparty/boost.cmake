include(FetchContent)
set(FETCHCONTENT_QUIET ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_TESTING OFF)
find_package(Git REQUIRED)

FetchContent_Declare(
        boost
        GIT_REPOSITORY "https://github.com/boostorg/boost.git"
        GIT_TAG boost-1.78.0
)
FetchContent_MakeAvailable(boost)
