include(FetchContent)
set(FETCHCONTENT_QUIET ON)
set(FETCHCONTENT_UPDATES_DISCONNECTED ON)
set(BUILD_SHARED_LIBS OFF)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)
find_package(Git REQUIRED)

set(BUILD_TESTING OFF)
set(ABSL_ENABLE_INSTALL ON)
set(ABSL_USE_EXTERNAL_GOOGLETEST ON)
FetchContent_Declare(
        absl
        GIT_REPOSITORY "https://github.com/abseil/abseil-cpp.git"
        GIT_TAG 20211102.0
)
FetchContent_MakeAvailable(absl)