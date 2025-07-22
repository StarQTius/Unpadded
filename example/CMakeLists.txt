find_program(DIFF_COMMAND diff
             DOC "'diff' Unix command" REQUIRED)

add_executable(dynamixel_protocol_20 dynamixel-protocol-2.0.cpp)
target_link_libraries(dynamixel_protocol_20 PRIVATE Unpadded::UnpaddedDebug)

add_custom_target(
  check_examples
  COMMAND dynamixel_protocol_20
  COMMAND ${DIFF_COMMAND} dynamixel_protocol_20.txt -)

add_iwyu_target(
  iwyu_examples
  dynamixel_protocol_20)
add_dependencies(check_examples iwyu_examples)
