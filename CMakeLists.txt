cmake_minimum_required(VERSION 3.21)

include(GNUInstallDirs)
include(FetchContent)

project(Unpadded LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/tool)
option(${PROJECT_NAME}_INSTALL "Only make what is needed for installing" OFF)

if(NOT ${PROJECT_NAME}_INSTALL)
  FetchContent_Declare(Unity
                       GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity)
  FetchContent_Declare(
    pybind11
    GIT_REPOSITORY https://github.com/pybind/pybind11
    GIT_TAG v2.10.0)
  FetchContent_Declare(
    Boost
    GIT_REPOSITORY https://github.com/boostorg/boost
    GIT_SUBMODULES libs/core libs/assert libs/config libs/static_assert
                   libs/throw_exception tools/cmake)
endif()

add_subdirectory(include)

if(NOT ${PROJECT_NAME}_INSTALL)
  add_subdirectory(doc EXCLUDE_FROM_ALL)

  if(${PROJECT_NAME}_IS_TOP_LEVEL)
    include(CTest)

    add_subdirectory(test EXCLUDE_FROM_ALL)
  endif()
endif()

install(
  TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}Targets
  PUBLIC_HEADER DESTINATION include COMPONENT Development)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/upd DESTINATION include)
install(
  EXPORT ${PROJECT_NAME}Targets
  FILE ${PROJECT_NAME}Config.cmake
  NAMESPACE Unpadded::
  DESTINATION lib/cmake/${PROJECT_NAME})
