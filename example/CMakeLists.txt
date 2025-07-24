define_property(
  TARGET
  PROPERTY REFERENCE_OUTPUT
  BRIEF_DOCS "A text file which contains what the executable should output"
  FULL_DOCS "It is used to check that example snippets give the same output \
    regardless of code changes")

add_executable(upd_Dynamixel2 dynamixel2.cpp)
target_link_libraries(upd_Dynamixel2 PRIVATE Unpadded::UnpaddedDebug)
set_property(TARGET upd_Dynamixel2 PROPERTY REFERENCE_OUTPUT ${CMAKE_CURRENT_SOURCE_DIR}/Dynamixel2.txt)

find_program(DIFF_COMMAND diff
             DOC "'diff' Unix command" REQUIRED)
add_custom_target(examples)
get_directory_property(EXAMPLE_EXECUTABLES DIRECTORY ${PROJECT_SOURCE_DIR}/example BUILDSYSTEM_TARGETS)
add_dependencies(examples ${EXAMPLE_EXECUTABLES})
foreach(EXECUTABLE IN LISTS EXAMPLE_EXECUTABLES)
  get_target_property(REFERENCE_OUTPUT ${EXECUTABLE} REFERENCE_OUTPUT)
  add_test(${EXECUTABLE}
    COMMAND ${EXECUTABLE}
    COMMAND ${DIFF_COMMAND} ${EXPECTED_OUTPUT_FILE} -
  )
  set_property(TEST ${EXECUTABLE} PROPERTY LABELS examples)
endforeach()
