cmake_minimum_required(VERSION 3.10)

cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH THIS_DIR)

find_package(Git REQUIRED)

execute_process(
  COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
  OUTPUT_VARIABLE ROOT_DIR)
string(STRIP ${ROOT_DIR} ROOT_DIR)

execute_process(
  COMMAND ${GIT_EXECUTABLE} diff --name-only
  OUTPUT_FILE unstaged_files.txt)
file(STRINGS unstaged_files.txt UNSTAGED_FILES)

execute_process(
  COMMAND ${GIT_EXECUTABLE} diff --name-only --cached
  OUTPUT_FILE staged_files.txt)
file(STRINGS staged_files.txt STAGED_FILES)

set(FILES ${STAGED_FILES} ${UNSTAGED_FILES})
set(CPP_FILES ${FILES})
list(FILTER CPP_FILES INCLUDE REGEX "^.*\\.(cpp|hpp)$")

foreach(FILE IN LISTS CPP_FILES)
  execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_MODULE_PATH=${THIS_DIR}/module -P ${THIS_DIR}/script/run-clang-format.cmake -- SOURCE_FILE ${ROOT_DIR}/${FILE})
endforeach()
