include(Paths)

# `script_as_command(<OUTPUT> <SCRIPT>)`
#
# Take a CMake script, build the corresponding command to invoke it and store it
# in `OUTPUT`
function(script_as_command OUTPUT SCRIPT)
  set(${OUTPUT}
      ${CMAKE_COMMAND} -DCMAKE_MODULE_PATH=${MODULE_PATH} -P ${SCRIPT} --
      PARENT_SCOPE)
endfunction()
