FetchContent_MakeAvailable(Unity)

function(add_cpp11_and_cpp17_test TEST_NAME)
  add_executable(run_${TEST_NAME}_cpp11 ${TEST_NAME}.cpp)
  set_target_properties(run_${TEST_NAME}_cpp11 PROPERTIES CXX_STANDARD 11)
  target_link_libraries(run_${TEST_NAME}_cpp11 PRIVATE unit_testing)
  add_test(NAME ${TEST_NAME}_cpp11 COMMAND run_${TEST_NAME}_cpp11)

  add_executable(run_${TEST_NAME}_cpp17 ${TEST_NAME}.cpp)
  set_target_properties(run_${TEST_NAME}_cpp17 PROPERTIES CXX_STANDARD 17)
  target_link_libraries(run_${TEST_NAME}_cpp17 PRIVATE unit_testing)
  add_test(NAME ${TEST_NAME}_cpp17 COMMAND run_${TEST_NAME}_cpp17)
endfunction()

add_library(unit_testing INTERFACE)
target_compile_options(
  unit_testing
  INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall
    -Werror>
    $<$<CXX_COMPILER_ID:GNU>:
    -fdiagnostics-color=always>
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
    -fcolor-diagnostics>)
target_link_libraries(unit_testing INTERFACE unity::framework ${PROJECT_NAME})

add_cpp11_and_cpp17_test(buffered_dispatcher)
add_cpp11_and_cpp17_test(dispatcher)
add_cpp11_and_cpp17_test(key_base)
add_cpp11_and_cpp17_test(key)
add_cpp11_and_cpp17_test(keyring)
add_cpp11_and_cpp17_test(order)
