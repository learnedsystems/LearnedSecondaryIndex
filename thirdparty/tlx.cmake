include(FetchContent)

set(TLX tlx)
FetchContent_Declare(
  ${TLX}
  GIT_REPOSITORY https://github.com/tlx/tlx
  GIT_TAG b6af589
  )

FetchContent_MakeAvailable(${TLX})
