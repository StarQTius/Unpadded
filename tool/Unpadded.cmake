if(NOT ${PROJECT_NAME}_INSTALL)
  FetchContent_MakeAvailable(pybind11)
endif()

if(NOT TARGET iwyu)
  find_program(IWYU_PROGRAM include-what-you-use)
  if(NOT IWYU_PROGRAM)
    message(
      WARNING
        "`include-what-you-use` program not found, skipping inclusions check")
  else()
    set(IWYU_COMMAND
        ${IWYU_PROGRAM} -std=gnu++17 -Xiwyu --no_fwd_decls -Xiwyu --error -Xiwyu
        --max_line_length=120 -Xiwyu
        --mapping_file=${PROJECT_SOURCE_DIR}/tool/.imp
        CACHE INTERNAL "IWYU command run for each IWYU target")
  endif()

  add_custom_target(iwyu)
endif()

function(add_iwyu_target FILE_PATH DEPENDENCY)
  if(IWYU_PROGRAM)
    # Find the relative path of the file from project root to make a shorter
    # target name and comment logs
    file(RELATIVE_PATH FILE_PATH_FROM_ROOT ${PROJECT_SOURCE_DIR}
         ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH})
    string(REPLACE "/" "_" TARGET_NAME ${FILE_PATH_FROM_ROOT})

    # Add the command and its corresponding target
    add_custom_command(
      OUTPUT ${CMAKE_BINARY_DIR}/${TARGET_NAME}.iwyu
      DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}
              ${PROJECT_SOURCE_DIR}/tool/.imp
      COMMAND touch ${CMAKE_BINARY_DIR}/${TARGET_NAME}.iwyu
      COMMAND
        for INCLUDE_PATH in $<TARGET_PROPERTY:${DEPENDENCY},INCLUDE_DIRECTORIES>
        ${pybind11_INCLUDE_DIRS} \\\; do
        INCLUDE_PATH_OPTION=\"$$INCLUDE_PATH_OPTION -isystem$$INCLUDE_PATH\"\\\;
        done\\\; ${IWYU_COMMAND} -I${CMAKE_CURRENT_SOURCE_DIR}
        $$INCLUDE_PATH_OPTION $$COMPILE_DEF_OPTION
        ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH} 2> iwyu_log \\\; if [ $$? -ne 1
        ]\\\; then exit 0\\\; else cat iwyu_log && printf %\"$$\(tput cols\)\"s
        | tr ' ' '='\\\; exit 1\\\; fi
      COMMAND_EXPAND_LISTS
      IMPLICIT_DEPENDS CXX ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH}
      COMMENT "Checking ${FILE_PATH_FROM_ROOT} inclusions")
    add_custom_target(
      IWYU_${TARGET_NAME}
      DEPENDS ${CMAKE_BINARY_DIR}/${TARGET_NAME}.iwyu
      COMMENT "")
    add_custom_target(IWYU_${TARGET_NAME}_group DEPENDS IWYU_${TARGET_NAME})

    # Add the include paths so CMake can find the file dependencies
    set_target_properties(
      IWYU_${TARGET_NAME}
      PROPERTIES INCLUDE_DIRECTORIES
                 $<TARGET_PROPERTY:${DEPENDENCY},INCLUDE_DIRECTORIES>)

    add_dependencies(iwyu IWYU_${TARGET_NAME})
  endif()
endfunction()
