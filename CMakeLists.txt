cmake_minimum_required(VERSION 3.21)

project(Unpadded LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/module)

if(PROJECT_IS_TOP_LEVEL)
  cmake_minimum_required(VERSION 3.27)

  include(FetchContent)
  include(Unpadded)

  upd_script_as_command(UPD_CLANG_FORMAT_LINTER_COMMAND ${PROJECT_SOURCE_DIR}/cmake/script/run-clang-format.cmake)
  upd_add_lint_target(clang_format "Check source file format" ${UPD_CLANG_FORMAT_LINTER_COMMAND} DRY_RUN YES)

  upd_script_as_command(UPD_IWYU_LINTER_COMMAND ${PROJECT_SOURCE_DIR}/cmake/script/run-include-what-you-use.cmake)
  upd_add_lint_target(iwyu "Check #includes consistency" ${UPD_IWYU_LINTER_COMMAND})

  upd_script_as_command(UPD_CLANG_TIDY_LINTER_COMMAND ${PROJECT_SOURCE_DIR}/cmake/script/run-clang-tidy.cmake)
  upd_add_lint_target(clang_tidy "Analyze source code" ${UPD_CLANG_TIDY_LINTER_COMMAND} CONFIG_FILE ${PROJECT_SOURCE_DIR}/.clang-tidy)

endif()

add_subdirectory(include EXCLUDE_FROM_ALL)
add_library(Unpadded::Unpadded ALIAS upd_Unpadded)
add_library(Unpadded::UnpaddedDebug ALIAS upd_UnpaddedDebug)

if(PROJECT_IS_TOP_LEVEL)
  FetchContent_Declare(
    Catch2
    GIT_REPOSITORY https://github.com/catchorg/Catch2
    GIT_TAG v3.8.1)
  list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)

  FetchContent_Declare(
    FakeIt
    GIT_REPOSITORY https://github.com/eranpeer/FakeIt
    GIT_TAG 2.4.1)

  add_subdirectory(example EXCLUDE_FROM_ALL)
  add_executable(Unpadded::Dynamixel2 ALIAS upd_Dynamixel2)

  add_subdirectory(test EXCLUDE_FROM_ALL)
  upd_add_test_suite(check
    "Check quickly for basic regressions"
    unitary)
  upd_add_test_suite(check_full
    "Check fully the whole project"
    unitary examples)
  upd_add_test_suite(check_unitary
    "Run unit tests"
    unitary)
  upd_add_test_suite(check_examples
    "Check whether example program outputs are stable"
    examples) 
endif()
