include(Paths)
include(Linting)

# `add_iwyu_target(<NAME> <LINTABLE_TARGET>)`
#
# Create the lint target `NAME` that lints `LINTABLE_TARGET` with Include What
# You Use
function(add_iwyu_target NAME LINTABLE_TARGET)
  script_as_command(IWYU_LINTER_COMMAND ${IWYU_LINTER_SCRIPT})
  add_lint_target(${NAME} ${LINTABLE_TARGET} ${IWYU_LINTER_COMMAND})
endfunction()
