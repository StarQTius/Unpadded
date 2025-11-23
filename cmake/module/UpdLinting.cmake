include(UpdGenex)
include(UpdScriptAsCommand)

set(UPD_LINTING_PROPERTIES
    INCLUDE_DIRECTORIES
    INTERFACE_INCLUDE_DIRECTORIES
    COMPILE_DEFINITIONS
    INTERFACE_COMPILE_DEFINITIONS
    COMPILE_FEATURES
    INTERFACE_COMPILE_FEATURES
    COMPILE_OPTIONS
    INTERFACE_COMPILE_OPTIONS)

define_property(
  TARGET
  PROPERTY UPD_LINT_COMPLETION_MARKERS
  BRIEF_DOCS "List of files which indicates linting operation completion"
  FULL_DOCS "This property is only populated for lint subtargets.")

define_property(
  TARGET
  PROPERTY UPD_LINTER_COMMAND
  BRIEF_DOCS "Command to invoke when linting"
  FULL_DOCS
    "The command is run when a target created by `upd_add_lint_target()` is built.")

define_property(
  TARGET
  PROPERTY UPD_LINTER_COMMENT
  BRIEF_DOCS "Prepended comment when a file is linted")

define_property(
  TARGET
  PROPERTY UPD_QUICK_LINT_TARGET
  BRIEF_DOCS "Corresponding quick lint target, if any"
  FULL_DOCS
    "Quick lint targets only lint source files that has been directly touched regardless of whether their dependencies has been touched.")

# `upd_target_lintable(<TARGET> <LINTABLE> <INHERIT_FROM>)`
#
# Add source file `LINTABLE` to lint to target `TARGET`
#
# Lint properties of target `INHERIT_FROM` will be provided to the lint
# command.
function(upd_target_lintable TARGET LINTABLE INHERIT_FROM IS_PRIVATE)
  cmake_path(ABSOLUTE_PATH LINTABLE)
  upd_properties_genex(LINTING_PROPERTIES_GENEX ${INHERIT_FROM} ${UPD_LINTING_PROPERTIES})

  cmake_path(RELATIVE_PATH LINTABLE OUTPUT_VARIABLE LINTABLE_RELATIVE_PATH)
  cmake_path(ABSOLUTE_PATH LINTABLE_RELATIVE_PATH BASE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_VARIABLE COMPLETION_MARKER)
  cmake_path(APPEND_STRING COMPLETION_MARKER .${TARGET})

  add_custom_command(
    OUTPUT ${COMPLETION_MARKER}_full
    COMMAND $<TARGET_PROPERTY:${TARGET},UPD_LINTER_COMMAND> SOURCE_FILE ${LINTABLE}
    ${LINTING_PROPERTIES_GENEX} CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR} $<$<BOOL:${IS_PRIVATE}>:PRIVATE_SOURCE>
            COMMAND ${CMAKE_COMMAND} -E make_directory $<PATH:GET_PARENT_PATH,${COMPLETION_MARKER}_full>
            COMMAND ${CMAKE_COMMAND} -E touch ${COMPLETION_MARKER}_full
            COMMAND ${CMAKE_COMMAND} -E touch ${COMPLETION_MARKER}_quick
    IMPLICIT_DEPENDS CXX ${LINTABLE}
    COMMENT "$<TARGET_PROPERTY:${TARGET},UPD_LINTER_COMMENT> -- ${LINTABLE_RELATIVE_PATH}"
    VERBATIM COMMAND_EXPAND_LISTS)

  # Subtarget is created in each directory since `add_custom_command()` output
  # cannot be depended from if target belongs to another directory
  set(SUBTARGET ${CMAKE_CURRENT_SOURCE_DIR}/${TARGET})
  string(REPLACE / _ SUBTARGET ${SUBTARGET})
  if(NOT TARGET ${SUBTARGET}_full)
    add_custom_target(
      ${SUBTARGET}_full
      DEPENDS $<TARGET_PROPERTY:${SUBTARGET}_full,UPD_LINT_COMPLETION_MARKERS>)
    add_dependencies(${TARGET} ${SUBTARGET}_full)
  endif()

  # Scan of files listed in `IMPLICIT_DEPENDS` uses `INCLUDE_DIRECTORIES`
  # property of dependent target to find header included with `#include<...>` .
  # Therefore, we have to copy this property from `INHERIT_FROM` so that the
  # CMake dependecy scanner find those headers.
  set_property(
    TARGET ${SUBTARGET}_full
    PROPERTY INCLUDE_DIRECTORIES
    $<TARGET_PROPERTY:${INHERIT_FROM},INCLUDE_DIRECTORIES>)

  set_property(
    TARGET ${SUBTARGET}_full
    APPEND
    PROPERTY UPD_LINT_COMPLETION_MARKERS ${COMPLETION_MARKER}_full)

  get_property(QUICK_LINT_TARGET
    TARGET ${TARGET}
    PROPERTY UPD_QUICK_LINT_TARGET)

  if(NOT DEFINED QUICK_LINT_TARGET)
    return()
  endif()

  add_custom_command(
    OUTPUT ${COMPLETION_MARKER}_quick
    COMMAND $<TARGET_PROPERTY:${TARGET},UPD_LINTER_COMMAND> SOURCE_FILE ${LINTABLE}
    ${LINTING_PROPERTIES_GENEX} CMAKE_BINARY_DIR ${CMAKE_BINARY_DIR} $<$<BOOL:${IS_PRIVATE}>:PRIVATE_SOURCE>
            COMMAND ${CMAKE_COMMAND} -E make_directory $<PATH:GET_PARENT_PATH,${COMPLETION_MARKER}_quick>
            COMMAND ${CMAKE_COMMAND} -E touch ${COMPLETION_MARKER}_full
            COMMAND ${CMAKE_COMMAND} -E touch ${COMPLETION_MARKER}_quick
    DEPENDS ${LINTABLE}
    COMMENT "$<TARGET_PROPERTY:${TARGET},UPD_LINTER_COMMENT> -- ${LINTABLE_RELATIVE_PATH}"
    VERBATIM COMMAND_EXPAND_LISTS)

  if(NOT TARGET ${SUBTARGET}_quick)
    add_custom_target(
      ${SUBTARGET}_quick
      DEPENDS $<TARGET_PROPERTY:${SUBTARGET}_quick,UPD_LINT_COMPLETION_MARKERS>)
    add_dependencies(${QUICK_LINT_TARGET} ${SUBTARGET}_quick)
  endif()

  set_property(
    TARGET ${SUBTARGET}_quick
    APPEND
    PROPERTY UPD_LINT_COMPLETION_MARKERS ${COMPLETION_MARKER}_quick)
endfunction()
