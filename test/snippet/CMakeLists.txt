FetchContent_MakeAvailable(ArduinoFake)

# Added so the compiler shuts up about the use of `std::uncaught_exception` in
# `ArduinoFake`
target_compile_options(
  ArduinoFake
  PUBLIC
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wno-deprecated-declarations>
)

add_executable(run_callee2 detail/callee2.cpp)
set_target_properties(run_callee2 PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_callee2 PRIVATE ${PROJECT_NAME})
add_test(NAME callee2 COMMAND run_callee2 PROPERTIES LABELS check)
add_dependencies(check run_callee2)

add_executable(run_caller2 detail/caller2.cpp)
set_target_properties(run_caller2 PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_caller2 PRIVATE ${PROJECT_NAME})
add_test(NAME caller2 COMMAND run_caller2 PROPERTIES LABELS check)
add_dependencies(check run_caller2)

add_executable(run_caller4 detail/caller4.cpp)
set_target_properties(run_caller4 PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_caller4 PRIVATE ${PROJECT_NAME})
add_test(NAME caller4 COMMAND run_caller4 PROPERTIES LABELS check)
add_dependencies(check run_caller4)

add_executable(run_caller5 detail/caller5.cpp)
set_target_properties(run_caller5 PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_caller5 PRIVATE ${PROJECT_NAME})
add_test(NAME caller5 COMMAND run_caller5 PROPERTIES LABELS check)
add_dependencies(check run_caller5)

add_executable(run_master1 master1.cpp)
set_target_properties(run_master1 PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_master1 PRIVATE ${PROJECT_NAME})
add_test(NAME master1 COMMAND run_master1 PROPERTIES LABELS check)
add_dependencies(check run_master1)

add_executable(run_shared1 shared1.cpp)
set_target_properties(run_shared1 PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_shared1 PRIVATE ${PROJECT_NAME})
add_test(NAME shared1 COMMAND run_shared1 PROPERTIES LABELS check)
add_dependencies(check run_shared1)

add_executable(run_slave1 slave1.cpp)
set_target_properties(run_slave1 PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_slave1 PRIVATE ${PROJECT_NAME})
target_compile_definitions(run_slave1 PRIVATE CALLEE)
add_test(NAME slave1 COMMAND run_slave1 PROPERTIES LABELS check)
add_dependencies(check run_slave1)

add_executable(run_serialization detail/serialization.cpp)
set_target_properties(run_serialization PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_serialization PRIVATE ${PROJECT_NAME})
target_compile_definitions(run_serialization PRIVATE CALLEE)
add_test(NAME serialization COMMAND run_serialization PROPERTIES LABELS check)
add_dependencies(check run_serialization)

add_custom_command(
  COMMAND ${CMAKE_COMMAND} -DREADME_PATH=${PROJECT_SOURCE_DIR}/README.md -P
          ${CMAKE_CURRENT_SOURCE_DIR}/parse_readme.cmake
  OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/caller.cpp
         ${CMAKE_CURRENT_BINARY_DIR}/callee.cpp
  DEPENDS ${PROJECT_SOURCE_DIR}/README.md)

add_executable(run_readme_caller detail/readme_caller.cpp
                                 ${CMAKE_CURRENT_BINARY_DIR}/caller.cpp)
set_target_properties(run_readme_caller PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_readme_caller PRIVATE ${PROJECT_NAME} ArduinoFake)
target_compile_definitions(run_readme_caller PRIVATE CALLEE)
add_test(NAME readme_caller COMMAND run_readme_caller PROPERTIES LABELS check)
add_dependencies(check run_readme_caller)

add_executable(run_readme_callee detail/readme_callee.cpp
                                 ${CMAKE_CURRENT_BINARY_DIR}/callee.cpp)
set_target_properties(run_readme_callee PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_readme_callee PRIVATE ${PROJECT_NAME} ArduinoFake)
target_compile_definitions(run_readme_callee PRIVATE CALLEE)
add_test(NAME readme_callee COMMAND run_readme_callee PROPERTIES LABELS check)
add_dependencies(check run_readme_callee)
