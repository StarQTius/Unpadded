function(add_iwyu_target FILE_PATH)
  string(REPLACE "/" "_" TARGET_SUFFIX ${FILE_PATH})
  add_custom_command(
    OUTPUT ${TARGET_SUFFIX}.iwyu
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}
    COMMAND touch ${TARGET_SUFFIX}.iwyu
    COMMAND
      ${CMAKE_CXX_INCLUDE_WHAT_YOU_USE} -std=c++17
      ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}\; if [ $$? -eq 2 ]\; then
      exit 0\; else exit 1\; fi)
  add_custom_target(Unpadded_IWYU_${TARGET_SUFFIX} ALL
                    DEPENDS ${TARGET_SUFFIX}.iwyu)
endfunction()
