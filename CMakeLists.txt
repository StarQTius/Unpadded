cmake_minimum_required(VERSION 3.21)

project(
  Unpadded
  VERSION 0.0
  LANGUAGES CXX)

include(FetchContent)

set(UNPADDED_BOOST_MODULES mp11 type_traits config static_assert)
set(UNPADDED_BOOST_LIBRARIES_DIRECTORY ${UNPADDED_BOOST_MODULES})
list(TRANSFORM UNPADDED_BOOST_LIBRARIES_DIRECTORY PREPEND libs/)
FetchContent_Declare(
  Boost
  GIT_REPOSITORY https://github.com/boostorg/boost
  GIT_SUBMODULES ${UNPADDED_BOOST_LIBRARIES_DIRECTORY} tools/cmake)

FetchContent_Declare(Unity
                     GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity)

if(Unpadded_IS_TOP_LEVEL AND DEFINED CMAKE_CXX_INCLUDE_WHAT_YOU_USE)
  set(FETCHCONTENT_FULLY_DISCONNECTED ON)
endif()

add_subdirectory(include)

if(Unpadded_IS_TOP_LEVEL AND BUILD_TESTING)
  include(CTest)
  add_subdirectory(test)
endif()
