include(Paths)
include(Linting)

# `add_iwyu_target(<NAME> <LINTABLE_TARGET>)`
#
# Create the lint target `NAME` that lints `LINTABLE_TARGET` with Include What
# You Use
function(add_iwyu_target NAME LINTABLE_TARGET)
  cmake_path(APPEND IWYU_LINTER_SCRIPT ${PROJECT_SOURCE_DIR} cmake script
             run-include-what-you-use.cmake)
  script_as_command(IWYU_LINTER_COMMAND "${IWYU_LINTER_SCRIPT}")
  add_lint_target(${NAME} ${LINTABLE_TARGET} ${IWYU_LINTER_COMMAND})
  target_inherited_lintables(${NAME} ${ARGN})
endfunction()
