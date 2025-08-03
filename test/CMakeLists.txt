FetchContent_MakeAvailable(Catch2)
FetchContent_MakeAvailable(FakeIt)

include(Catch)
include(CTest)
include(Unpadded)

add_executable(upd_unitary
  named_tuple_v2.cpp)
target_link_libraries(upd_unitary PRIVATE
  Unpadded::UnpaddedDebug
  Catch2::Catch2WithMain
  FakeIt::FakeIt-catch
)
catch_discover_tests(upd_unitary PROPERTIES LABELS unitary)

script_as_command(RUN_AND_COMPARE_COMMAND ${PROJECT_SOURCE_DIR}/cmake/script/run-and-compare.cmake)
add_custom_target(upd_examples)
add_dependencies(upd_examples Unpadded::Dynamixel2)
get_property(EXAMPLE_EXECUTABLES TARGET upd_examples PROPERTY MANUALLY_ADDED_DEPENDENCIES)
foreach(EXECUTABLE IN LISTS EXAMPLE_EXECUTABLES)
  set(TEST_NAME "Run and compare ${EXECUTABLE}")
  add_test(
    NAME ${TEST_NAME}
    COMMAND ${RUN_AND_COMPARE_COMMAND}
      $<TARGET_FILE:${EXECUTABLE}>
      ${CMAKE_CURRENT_SOURCE_DIR}/$<TARGET_FILE_NAME:${EXECUTABLE}>.txt)
  set_property(TEST ${TEST_NAME} PROPERTY LABELS examples)
endforeach()

set(EXECUTABLE upd_Dynamixel2)

# `upd_add_test_suite(<NAME> <COMMENT> <LABELS...>)`
#
# Create the target `NAME` that runs the tests under the labels `LABELS...`
#
# Those labels, when prefixed with 'upd_', must also name valid targets.
function(upd_add_test_suite NAME COMMENT)
  set(CTEST_FLAGS --output-on-failure)
  cmake_path(APPEND TEST_PATH ${PROJECT_BINARY_DIR} test)

  set(LABELS ${ARGN})
  list(TRANSFORM LABELS PREPEND upd_ OUTPUT_VARIABLE TARGETS)

  add_custom_target(${NAME}
    COMMAND ctest ${CTEST_FLAGS} -L ${LABELS}
    WORKING_DIRECTORY ${TEST_PATH}
    DEPENDS ${TARGETS}
    COMMENT ${COMMENT})
endfunction()
