FetchContent_MakeAvailable(Catch2)
FetchContent_MakeAvailable(FakeIt)

include(Catch)
include(CTest)
include(Unpadded)

add_executable(unitary
  named_tuple_v2.cpp)
target_link_libraries(unitary PRIVATE
  Unpadded::UnpaddedDebug
  Catch2::Catch2WithMain
  FakeIt::FakeIt-catch
)
catch_discover_tests(unitary PROPERTIES LABELS unitary)

find_program(DIFF_COMMAND diff
             DOC "'diff' Unix command" REQUIRED)
foreach(EXECUTABLE IN LISTS EXAMPLE_EXECUTABLES)
  get_target_property(REFERENCE_OUTPUT ${EXECUTABLE} REFERENCE_OUTPUT)
  add_test(${EXECUTABLE}
    COMMAND ${EXECUTABLE}
    COMMAND ${DIFF_COMMAND} ${EXPECTED_OUTPUT_FILE} -
  )
  set_property(TEST ${EXECUTABLE} PROPERTY LABELS examples)
endforeach()

# `upd_add_test_suite(<NAME> <COMMENT> <LABELS...>)`
#
# Create the target `NAME` that runs the tests under the labels `LABELS...`
#
# Those labels must also name valid targets.
function(upd_add_test_suite NAME COMMENT)
  set(CTEST_FLAGS --output-on-failure)
  cmake_path(APPEND TEST_PATH ${PROJECT_BINARY_DIR} test)

  add_custom_target(${NAME}
    COMMAND ctest ${CTEST_FLAGS} -L ${ARGN}
    WORKING_DIRECTORY ${TEST_PATH}
    DEPENDS ${ARGN}
    COMMENT ${COMMENT})
endfunction()
