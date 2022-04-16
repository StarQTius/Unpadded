cmake_minimum_required(VERSION 3.21)

project(Key-To-Order LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/tool)
include(${PROJECT_NAME})

include(FetchContent)

FetchContent_Declare(Unpadded
                     GIT_REPOSITORY https://github.com/StarQTius/Unpadded)

FetchContent_Declare(Unity
                     GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity)

if(DEFINED CMAKE_CXX_INCLUDE_WHAT_YOU_USE)
  set(FETCHCONTENT_FULLY_DISCONNECTED ON)
endif()

add_subdirectory(include)
add_subdirectory(doc)

if(${PROJECT_NAME}_IS_TOP_LEVEL AND BUILD_TESTING)
  include(CTest)

  add_subdirectory(test)
endif()
