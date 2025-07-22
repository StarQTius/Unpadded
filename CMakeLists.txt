cmake_minimum_required(VERSION 3.27)

include(FetchContent)

project(Unpadded LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/module)

add_subdirectory(include)

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

  add_subdirectory(test EXCLUDE_FROM_ALL)
  add_subdirectory(example EXCLUDE_FROM_ALL)
endif()
