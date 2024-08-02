# `normalize_path(<PATH_VAR>)`
#
# Normalize `PATH_VAR` so that it is expressed as an absolute CMake-style path
#
# If `PATH_VAR` is relative, it is considered relative to the current source
# directory.
function(normalize_path PATH_VAR)
  set(RETVAL ${${PATH_VAR}})
  cmake_path(SET RETVAL NORMALIZE ${RETVAL})
  cmake_path(IS_RELATIVE RETVAL IS_INPUT_RELATIVE)

  if(IS_INPUT_RELATIVE)
    cmake_path(ABSOLUTE_PATH RETVAL)
  endif()

  set(${PATH_VAR}
      ${RETVAL}
      PARENT_SCOPE)
endfunction()
