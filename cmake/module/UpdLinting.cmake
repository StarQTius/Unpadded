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
  FULL_DOCS "A target created by `upd_add_lint_target()` will depend on these \
    files. See `upd_add_lint_target()` and `upd_target_lintables()` respective \
    documentations to learn how this property is populated.")

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

# `upd_add_lint_target(<NAME> <LINTABLE_TARGET> <COMMENT> <COMMAND...>)`
#
# Create the target `NAME` that lints the executable or library
# `LINTABLE_TARGET` with `COMMAND...`
#
# When `NAME` is built, `COMMAND...` is invoked on each source file of
# `LINTABLE_TARGET` that currently appears in its `SOURCES` property (sources
# added after will NOT be linted if not added with `target_lintables()`).
# `COMMAND...` is stored in the `UPD_LINTER_COMMAND` property of `NAME`.
#
# Linting-related properties of `LINTABLE_TARGET` are transfered to `NAME`
# (those are listed in `UPD_LINTING_PROPERTIES`).
#
# `COMMENT` shell be displayed each time a file is linted.
#
# See `upd_target_lintable()` for more information.
function(upd_add_lint_target NAME LINTABLE_TARGET COMMENT)
  add_custom_target(${NAME}
    DEPENDS $<TARGET_PROPERTY:${NAME},UPD_LINT_COMPLETION_MARKERS>)

  set_property(TARGET ${NAME} PROPERTY UPD_LINTER_COMMENT ${COMMENT})
  set_property(TARGET ${NAME} PROPERTY UPD_LINTER_COMMAND ${ARGN})
  foreach(PROP IN LISTS UPD_LINTING_PROPERTIES)
    set_property(TARGET ${NAME}
                 PROPERTY ${PROP} $<TARGET_PROPERTY:${LINTABLE_TARGET},${PROP}>)
  endforeach()

  get_property(
    TARGET_SOURCES
    TARGET ${LINTABLE_TARGET}
    PROPERTY SOURCES)
  upd_target_lintables(${NAME} ${TARGET_SOURCES})
endfunction()

# `upd_target_lintables(<TARGET> <SOURCE_FILES...>)`
#
# Add source files to lint to a target `TARGET` created with `upd_add_lint_target()`
#
# The explicit command is `<TARGET UPD_LINTER_COMMAND value> <source file>
# (<linting-related property name> <TARGET linting-related property value>)...`.
function(upd_target_lintables TARGET)
  foreach(LINTABLE IN LISTS ARGN)
    upd_target_lintable(${TARGET} ${LINTABLE} ${TARGET})
  endforeach()
endfunction()

# `upd_target_inherited_lintables(<TARGET> <EXECUTABLE_TARGETS...>)`
#
# Let `TARGET` be responsible for linting each source file of `EXECUTABLE_TARGETS...`
function(upd_target_inherited_lintables TARGET)
  foreach(EXECUTABLE_TARGET IN LISTS ARGN)
    get_property(
      TARGET_SOURCES
      TARGET ${EXECUTABLE_TARGET}
      PROPERTY SOURCES)

    foreach(SOURCE IN LISTS TARGET_SOURCES)
      upd_target_lintable(${TARGET} ${SOURCE} ${EXECUTABLE_TARGET})
    endforeach()
  endforeach()
endfunction()

# `upd_target_lintable(<TARGET> <LINTABLE> [<INHERIT_FROM>])`
#
# Add source file `LINTABLE` to lint to target `TARGET`. If `INHERIT_FROM` is set, lint properties of target `INHERIT_FROM` will be provided to lint command instead of whose of `TARGET`.
function(upd_target_lintable TARGET LINTABLE INHERIT_FROM)
  upd_properties_genex(LINTING_PROPERTIES_GENEX ${INHERIT_FROM}
                   ${LINTING_PROPERTIES})
  upd_is_source_of_target_genex(IS_LINTABLE_SOURCE_GENEX ${INHERIT_FROM}
                            ${LINTABLE})

  cmake_path(ABSOLUTE_PATH LINTABLE)
  cmake_path(RELATIVE_PATH LINTABLE OUTPUT_VARIABLE LINTABLE_RELATIVE_PATH)
  cmake_path(ABSOLUTE_PATH LINTABLE BASE_DIRECTORY
             ${PROJECT_BINARY_DIR} OUTPUT_VARIABLE COMPLETION_MARKER)
  cmake_path(APPEND_STRING COMPLETION_MARKER .${TARGET})

  add_custom_command(
    OUTPUT ${COMPLETION_MARKER}
    COMMAND $<TARGET_PROPERTY:${TARGET},UPD_LINTER_COMMAND> ${LINTABLE}
            ${LINTING_PROPERTIES_GENEX} $<${IS_LINTABLE_SOURCE_GENEX}:IS_SOURCE>
            COMMAND ${CMAKE_COMMAND} -E touch ${COMPLETION_MARKER}
    IMPLICIT_DEPENDS CXX ${LINTABLE}
    COMMENT "$<TARGET_PROPERTY:${TARGET},UPD_LINTER_COMMENT> -- ${LINTABLE_RELATIVE_PATH}"
    VERBATIM COMMAND_EXPAND_LISTS)

  set_property(
    TARGET ${TARGET}
    APPEND
    PROPERTY UPD_LINT_COMPLETION_MARKERS ${COMPLETION_MARKER})
endfunction()
