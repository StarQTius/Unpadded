add_executable(upd_Dynamixel2 dynamixel2.cpp)
target_link_libraries(upd_Dynamixel2 PRIVATE Unpadded::UnpaddedDebug)
set_property(TARGET upd_Dynamixel2 PROPERTY OUTPUT_NAME Dynamixel2)

add_custom_target(examples)
add_dependencies(examples Unpadded::Dynamixel2)
