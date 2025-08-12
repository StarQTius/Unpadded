include(UpdLinting)

# `upd_add_lint_target(<NAME> <COMMENT> <COMMAND...>)`
#
# Create a target `NAME` which lint each file attached to it by
# `upd_target_lintables()`
#
# The `UPD_LINTER_COMMENT` property of this target will be populated with
# `COMMENT`.
# The `UPD_LINTER_COMMAND` property of this target will be populated with
# `COMMAND...`.
function(upd_add_lint_target NAME COMMENT)
  add_custom_target(${NAME})
  set_property(
    TARGET ${NAME}
    PROPERTY UPD_LINTER_COMMAND
    ${ARGN})
  set_property(
    TARGET ${NAME}
    PROPERTY UPD_LINTER_COMMENT
    ${COMMENT})
endfunction()

# `upd_target_lintables(<TARGET> <LINTABLE_TARGET> <EXTRA_SOURCES...>)`
#
# Attach source files to be linted from `LINTABLE_TARGET` to `TARGET`
#
# Compilation-related properties (listed in `UPD_LINTING_PROPERTIES`) from
# `LINTABLE_TARGET` will be used to lint each source file from
# `LINTABLE_TARGET` and `EXTRA_SOURCES...`.
function(upd_target_lintables TARGET LINTABLE_TARGET)
  get_property(
    SOURCES
    TARGET ${LINTABLE_TARGET}
    PROPERTY SOURCES)

  set(SUBTARGET ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET})
  string(REPLACE / _ SUBTARGET ${SUBTARGET})
  if(TARGET ${SUBTARGET})
    # Subtarget is created in each directory since `add_custom_command()` output
    # cannot be depended from if target belongs to another directory
    add_custom_target(
      ${SUBTARGET}
      DEPENDS $<TARGET_PROPERTY:${SUBTARGET},UPD_LINT_COMPLETION_MARKERS>)
  endif()

  foreach(SOURCE IN LISTS SOURCES ARGN)
    upd_target_lintable(${TARGET} ${SUBTARGET} ${SOURCE} ${LINTABLE_TARGET})
  endforeach()
  add_dependencies(${TARGET} ${SUBTARGET})
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
    COMMAND ${CMAKE_CTEST_COMMAND} ${CTEST_FLAGS} -L ${LABELS}
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
