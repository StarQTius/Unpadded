function(add_iwyu_target FILE_PATH DEPENDENCY)
  string(REPLACE "/" "_" TARGET_SUFFIX ${FILE_PATH})
  add_custom_command(
    OUTPUT ${TARGET_SUFFIX}.iwyu
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}
    COMMAND touch ${TARGET_SUFFIX}.iwyu
    COMMAND
      for INCLUDE_PATH in $<TARGET_PROPERTY:${DEPENDENCY},INTERFACE_INCLUDE_DIRECTORIES>\\\; do
        INCLUDE_PATH_OPTION=\"$$INCLUDE_PATH_OPTION -I$$INCLUDE_PATH\"\\\;
      done\\\;
      ${CMAKE_CXX_INCLUDE_WHAT_YOU_USE} -std=c++11 $$INCLUDE_PATH_OPTION
      ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}\\\; if [ $$? -eq 2 ]\\\; then
      exit 0\\\; else exit 1\\\; fi
    COMMAND_EXPAND_LISTS)
  add_custom_target(${PROJECT_NAME}_IWYU_${TARGET_SUFFIX} ALL
                    DEPENDS ${TARGET_SUFFIX}.iwyu)
endfunction()
