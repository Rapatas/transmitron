
set(BUILD_WITH_TIDY OFF CACHE BOOL "")

if (BUILD_WITH_TIDY)
  find_program(CLANG_TIDY_COMMAND NAMES "clang-tidy" REQUIRED)
  set(CMAKE_CXX_CLANG_TIDY "")
  list(APPEND CMAKE_CXX_CLANG_TIDY "clang-tidy")
  list(APPEND CMAKE_CXX_CLANG_TIDY "--config-file=${CMAKE_SOURCE_DIR}/.clang-tidy")
  list(APPEND CMAKE_CXX_CLANG_TIDY "-header-filter=${CMAKE_SOURCE_DIR}/src/.*.hpp")
endif()

