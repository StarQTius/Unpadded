# `upd_require_defined(<VARIABLE>)`
#
# Assert that `VARIABLE` is defined
macro(upd_require_defined VARIABLE)
  if(NOT DEFINED ${VARIABLE})
    message(FATAL_ERROR "`${VARIABLE}` should have been defined")
  endif()
endmacro()

# `upd_require_defined_with_message(<VARIABLE> <MESSAGE>)`
#
# Assert that `VARIABLE` is defined and show `MESSAGE` if not
macro(upd_require_defined_with_message VARIABLE MESSAGE)
  if(NOT DEFINED ${VARIABLE})
    message(FATAL_ERROR "${MESSAGE}")
  endif()
endmacro()
