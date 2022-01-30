cmake_minimum_required(VERSION 3.12)

message("Finding ccache...")
find_program(CCACHE_FOUND ccache)
if(CCACHE_FOUND)
  message(STATUS "ccache found")
  set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE ccache)
else()
  message(WARNING "ccache not found")
endif()

include(FetchContent)

project(Unpadded
  VERSION 0.0
  LANGUAGES CXX)
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(Unpadded_Boost_Modules mp11 type_traits config static_assert)
set(Unpadded_Boost_Libraries_Directory ${Unpadded_Boost_Modules})
list(TRANSFORM Unpadded_Boost_Libraries_Directory PREPEND libs/)

message("Making Boost available...")
FetchContent_Declare(Boost
  GIT_REPOSITORY https://github.com/boostorg/boost
  GIT_SUBMODULES ${Unpadded_Boost_Libraries_Directory} tools/cmake)
FetchContent_MakeAvailable(Boost)

add_subdirectory(include)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
  enable_testing()
  if(BUILD_TESTING)
    message("Making Unity available...")
    FetchContent_Declare(Unity
      GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity)
    FetchContent_MakeAvailable(Unity)

    message("Building test")
    add_subdirectory(test)
  endif()
endif()
