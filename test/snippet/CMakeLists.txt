FetchContent_MakeAvailable(ArduinoFake)

# Added so the compiler shuts up about the use of `std::uncaught_exception` in
# `ArduinoFake`
target_compile_options(
  ArduinoFake
  PUBLIC
    $<$<OR:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-Wno-deprecated-declarations>
)

function(add_snippet_test TEST_NAME SRC)
  add_executable(run_${TEST_NAME} ${SRC})
  set_target_properties(run_${TEST_NAME} PROPERTIES CXX_STANDARD 17)
  target_link_libraries(run_${TEST_NAME} PRIVATE ${PROJECT_NAME})
  add_test(NAME ${TEST_NAME} COMMAND run_${TEST_NAME})
  set_tests_properties(${TEST_NAME} PROPERTIES LABELS check)
  add_dependencies(check run_${TEST_NAME})
endfunction()

add_snippet_test(callee2 detail/callee2.cpp)
add_snippet_test(caller2 detail/caller2.cpp)
add_snippet_test(caller4 detail/caller4.cpp)
add_snippet_test(caller5 detail/caller5.cpp)
add_snippet_test(master1 master1.cpp)
add_snippet_test(shared1 shared1.cpp)
add_snippet_test(slave1 slave1.cpp)
add_snippet_test(serialization detail/serialization.cpp)

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
add_test(NAME readme_caller COMMAND run_readme_caller)
set_tests_properties(readme_caller PROPERTIES LABELS check)
add_dependencies(check run_readme_caller)

add_executable(run_readme_callee detail/readme_callee.cpp
                                 ${CMAKE_CURRENT_BINARY_DIR}/callee.cpp)
set_target_properties(run_readme_callee PROPERTIES CXX_STANDARD 17)
target_link_libraries(run_readme_callee PRIVATE ${PROJECT_NAME} ArduinoFake)
target_compile_definitions(run_readme_callee PRIVATE CALLEE)
add_test(NAME readme_callee COMMAND run_readme_callee)
set_tests_properties(readme_callee PROPERTIES LABELS check)
add_dependencies(check run_readme_callee)
