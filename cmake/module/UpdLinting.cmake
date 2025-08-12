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

# `upd_target_lintable(<TARGET> <LINTABLE> [<INHERIT_FROM>])`
#
# Add source file `LINTABLE` to lint to target `TARGET`. If `INHERIT_FROM` is set, lint properties of target `INHERIT_FROM` will be provided to lint command instead of whose of `TARGET`.
function(upd_target_lintable TARGET SUBTARGET LINTABLE INHERIT_FROM)
  cmake_path(ABSOLUTE_PATH LINTABLE)

  upd_properties_genex(LINTING_PROPERTIES_GENEX ${INHERIT_FROM} ${UPD_LINTING_PROPERTIES})
  upd_is_source_of_target_genex(IS_LINTABLE_SOURCE_GENEX ${INHERIT_FROM}
                            ${LINTABLE})

  cmake_path(RELATIVE_PATH LINTABLE OUTPUT_VARIABLE LINTABLE_RELATIVE_PATH)
  cmake_path(ABSOLUTE_PATH LINTABLE_RELATIVE_PATH BASE_DIRECTORY
    ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_VARIABLE COMPLETION_MARKER)
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
    TARGET ${SUBTARGET}
    APPEND
    PROPERTY UPD_LINT_COMPLETION_MARKERS ${COMPLETION_MARKER})
endfunction()
