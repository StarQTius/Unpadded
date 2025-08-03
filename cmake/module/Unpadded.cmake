include(UpdLinting)

# `upd_add_iwyu_target(<NAME> <LINTABLE_TARGET>)`
#
# Create the lint target `NAME` that lints `LINTABLE_TARGET` with Include What
# You Use
function(upd_add_iwyu_target NAME LINTABLE_TARGET)
  set(IWYU_LINTER_SCRIPT ${PROJECT_SOURCE_DIR}/cmake/script/run-include-what-you-use.cmake)
  upd_script_as_command(IWYU_LINTER_COMMAND ${IWYU_LINTER_SCRIPT})
  upd_add_lint_target(${NAME} ${LINTABLE_TARGET} "Check #includes consistency" ${IWYU_LINTER_COMMAND})
  upd_target_inherited_lintables(${NAME} ${ARGN})
endfunction()

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

# `upd_register_run_and_compare_tests(<GROUP_TARGET> <LABEL>)`
#
# Create a run-and-compare test for each executable depended by `GROUP_TARGET`
#
# Each test will be labeled with `LABEL`.
function(upd_register_run_and_compare_tests GROUP_TARGET LABEL)
  upd_script_as_command(RUN_AND_COMPARE_COMMAND ${PROJECT_SOURCE_DIR}/cmake/script/run-and-compare.cmake)
  get_property(EXAMPLE_EXECUTABLES TARGET ${GROUP_TARGET} PROPERTY MANUALLY_ADDED_DEPENDENCIES)
  foreach(EXECUTABLE IN LISTS EXAMPLE_EXECUTABLES)
    set(TEST_NAME "Run and compare ${EXECUTABLE}")
    add_test(
      NAME ${TEST_NAME}
      COMMAND ${RUN_AND_COMPARE_COMMAND}
        $<TARGET_FILE:${EXECUTABLE}>
        ${CMAKE_CURRENT_SOURCE_DIR}/$<TARGET_FILE_NAME:${EXECUTABLE}>.txt)
      set_property(TEST ${TEST_NAME} PROPERTY LABELS ${LABEL})
  endforeach()
endfunction()
