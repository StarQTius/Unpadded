FetchContent_MakeAvailable(Unity)

function(add_cpp11_and_cpp17_test TEST_NAME)
  add_executable(run_${TEST_NAME}_cpp11 ${TEST_NAME}.cpp)
  set_target_properties(run_${TEST_NAME}_cpp11 PROPERTIES CXX_STANDARD 11)
  target_link_libraries(run_${TEST_NAME}_cpp11 PRIVATE unit_testing)
  add_test(NAME ${TEST_NAME}_cpp11 COMMAND run_${TEST_NAME}_cpp11)
  set_tests_properties(${TEST_NAME}_cpp11 PROPERTIES LABELS check)

  add_executable(run_${TEST_NAME}_cpp17 ${TEST_NAME}.cpp)
  set_target_properties(run_${TEST_NAME}_cpp17 PROPERTIES CXX_STANDARD 17)
  target_link_libraries(run_${TEST_NAME}_cpp17 PRIVATE unit_testing)
  add_test(NAME ${TEST_NAME}_cpp17 COMMAND run_${TEST_NAME}_cpp17)
  set_tests_properties(${TEST_NAME}_cpp17 PROPERTIES LABELS check)

  add_dependencies(check run_${TEST_NAME}_cpp11 run_${TEST_NAME}_cpp17)
endfunction()

add_library(unit_testing INTERFACE)
target_compile_options(
  unit_testing
  INTERFACE -Wall -Werror $<$<CXX_COMPILER_ID:GNU>: -fdiagnostics-color=always>
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
            -fcolor-diagnostics>)
target_link_libraries(unit_testing INTERFACE unity::framework ${PROJECT_NAME})

add_custom_target(
  check
  COMMAND ctest -L check --output-on-failure
  DEPENDS module)

add_subdirectory(py)
add_subdirectory(snippet)

add_cpp11_and_cpp17_test(buffered_dispatcher)
add_cpp11_and_cpp17_test(dispatcher)
add_cpp11_and_cpp17_test(key)
add_cpp11_and_cpp17_test(keyring)
add_cpp11_and_cpp17_test(action)
add_cpp11_and_cpp17_test(tuple_view)
add_cpp11_and_cpp17_test(tuple)
add_cpp11_and_cpp17_test(unaligned_data)
