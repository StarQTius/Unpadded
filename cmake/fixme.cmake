cmake_minimum_required(VERSION 3.10)

cmake_path(GET CMAKE_SCRIPT_MODE_FILE PARENT_PATH THIS_DIR)

find_package(Git REQUIRED)

execute_process(
  COMMAND ${GIT_EXECUTABLE} rev-parse --show-toplevel
  OUTPUT_VARIABLE ROOT_DIR)
string(STRIP ${ROOT_DIR} ROOT_DIR)

execute_process(
  COMMAND ${GIT_EXECUTABLE} ls-files
  OUTPUT_FILE indexed_files.txt)
file(STRINGS indexed_files.txt INDEXED_FILES)
file(REMOVE indexed_files.txt)

execute_process(
  COMMAND ${GIT_EXECUTABLE} ls-files --others --exclude-standard
  OUTPUT_FILE untracked_files.txt)
file(STRINGS untracked_files.txt UNTRACKED_FILES)
file(REMOVE untracked_files.txt)

set(FILES ${INDEXED_FILES} ${UNTRACKED_FILES})
set(CPP_FILES ${FILES})
list(FILTER CPP_FILES INCLUDE REGEX "^.*\\.(cpp|hpp)$")

message("Formating C++ files")
foreach(FILE IN LISTS CPP_FILES)
  message(STATUS "Formatting ${FILE}")
  execute_process(
    COMMAND ${CMAKE_COMMAND} -DCMAKE_MODULE_PATH=${THIS_DIR}/module -P ${THIS_DIR}/script/run-clang-format.cmake -- SOURCE_FILE ${ROOT_DIR}/${FILE})
endforeach()
