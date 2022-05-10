include(FetchContent)

set(TLX tlx)
FetchContent_Declare(
  ${TLX}
  GIT_REPOSITORY https://github.com/tlx/tlx
  GIT_TAG fa1ee82
  )

FetchContent_MakeAvailable(${TLX})
