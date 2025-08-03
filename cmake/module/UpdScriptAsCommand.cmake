# `upd_script_as_command(<OUTPUT> <SCRIPT>)`
#
# Take a CMake script, build the corresponding command to invoke it and store it
# in `OUTPUT`
function(upd_script_as_command OUTPUT SCRIPT)
  cmake_path(NATIVE_PATH SCRIPT NORMALIZE SCRIPT_NATIVE_PATH)
  set(${OUTPUT}
    ${CMAKE_COMMAND} -DCMAKE_MODULE_PATH=${CMAKE_MODULE_PATH} -P ${SCRIPT_NATIVE_PATH} --
      PARENT_SCOPE)
endfunction()
