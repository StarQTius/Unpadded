include(NormalizePath)
include(Genex)
include(ScriptAsCommand)
include(Paths)

set(LINTING_PROPERTIES
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
  PROPERTY LINT_COMPLETION_MARKERS
  BRIEF_DOCS "List of files which indicates linting operation completion"
  FULL_DOCS "A target created by `add_lint_target()` will depend on these \
    files. See `add_lint_target()` and `target_lintables()` respective \
    documentations to learn how this property is populated.")

define_property(
  TARGET
  PROPERTY LINTER_COMMAND
  BRIEF_DOCS "Command to invoke when linting"
  FULL_DOCS
    "The command is run when a target created by `add_lint_target()` is built.")

# `add_lint_target(<NAME> <LINTABLE_TARGET> <COMMAND...>)`
#
# Create the target `NAME` that lints the executable or library
# `LINTABLE_TARGET` with `COMMAND...`
#
# When `NAME` is built, `COMMAND...` is invoked on each source file of
# `LINTABLE_TARGET` that currently appears in its `SOURCES` property (sources
# added after will NOT be linted if not added with `target_lintables()`).
# `COMMAND...` is stored in the `LINTER_COMMAND` property of `NAME`.
#
# Linting-related properties of `LINTABLE_TARGET` are transfered to `NAME`
# (those are listed in `LINTING_PROPERTIES`).
#
# See `target_lintable()` for more information.
function(add_lint_target NAME LINTABLE_TARGET)
  add_custom_target(${NAME}
                    DEPENDS $<TARGET_PROPERTY:${NAME},LINT_COMPLETION_MARKERS>)

  set_property(TARGET ${NAME} PROPERTY LINTER_COMMAND ${ARGN})
  foreach(PROP IN LISTS LINTING_PROPERTIES)
    set_property(TARGET ${NAME}
                 PROPERTY ${PROP} $<TARGET_PROPERTY:${LINTABLE_TARGET},${PROP}>)
  endforeach()

  get_property(
    TARGET_SOURCES
    TARGET ${LINTABLE_TARGET}
    PROPERTY SOURCES)
  target_lintables(${NAME} ${TARGET_SOURCES})
endfunction()

# `target_lintables(<TARGET> <SOURCE_FILES...>)`
#
# Add source files to lint to a target `TARGET` created with `add_lint_target()`
#
# The explicit command is `<TARGET LINTABLE_COMMAND value> <source file>
# (<linting-related property name> <TARGET linting-related property value>)...`.
function(target_lintables TARGET)
  foreach(LINTABLE IN LISTS ARGN)
    normalize_path(LINTABLE)
    properties_genex(LINTING_PROPERTIES_GENEX ${TARGET} ${LINTING_PROPERTIES})
    is_source_of_target_genex(IS_LINTABLE_SOURCE_GENEX ${TARGET} ${LINTABLE})
    script_as_command(TOUCH_COMMAND ${TOUCH_SCRIPT})

    cmake_path(RELATIVE_PATH LINTABLE BASE_DIRECTORY ${PROJECT_SOURCE_DIR}
               OUTPUT_VARIABLE LINTABLE_RELATIVE_PATH)
    cmake_path(ABSOLUTE_PATH LINTABLE_RELATIVE_PATH BASE_DIRECTORY
               ${PROJECT_BINARY_DIR} OUTPUT_VARIABLE COMPLETION_MARKER)
    cmake_path(APPEND_STRING COMPLETION_MARKER .${TARGET})

    add_custom_command(
      OUTPUT ${COMPLETION_MARKER}
      COMMAND
        $<TARGET_PROPERTY:${TARGET},LINTER_COMMAND> ${LINTABLE}
        ${LINTING_PROPERTIES_GENEX} $<${IS_LINTABLE_SOURCE_GENEX}:IS_SOURCE>
      COMMAND ${TOUCH_COMMAND} ${COMPLETION_MARKER}
      IMPLICIT_DEPENDS CXX ${LINTABLE}
      COMMENT "${TARGET} -- ${LINTABLE_RELATIVE_PATH}"
      VERBATIM COMMAND_EXPAND_LISTS)

    set_property(
      TARGET ${TARGET}
      APPEND
      PROPERTY LINT_COMPLETION_MARKERS ${COMPLETION_MARKER})
  endforeach()
endfunction()
