cmake_minimum_required(VERSION 3.21)

project(Unpadded LANGUAGES CXX)
list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/tool)

include(FetchContent)

FetchContent_Declare(Unity
                     GIT_REPOSITORY https://github.com/ThrowTheSwitch/Unity)

if(${PROJECT_NAME}_IS_TOP_LEVEL)
  set(FETCHCONTENT_FULLY_DISCONNECTED ON)
endif()

add_subdirectory(include)
add_subdirectory(doc)

if(${PROJECT_NAME}_IS_TOP_LEVEL AND BUILD_TESTING)
  include(CTest)

  add_subdirectory(test)
endif()
