include(FetchContent)
set(FETCHCONTENT_QUIET ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(Git REQUIRED)

set(BUILD_TESTING OFF)
FetchContent_Declare(
    tsl
    GIT_REPOSITORY "https://github.com/Tessil/robin-map.git"
    GIT_TAG v0.6.3)

FetchContent_MakeAvailable(tsl)
