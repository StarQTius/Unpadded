FetchContent_MakeAvailable(Unity)

function(add_cpp11_and_cpp17_test TEST_NAME)
  add_executable(run_${TEST_NAME}_cpp11 ${TEST_NAME}.cpp)
  target_link_libraries(run_${TEST_NAME}_cpp11 PRIVATE unit_testing_cpp11)
  add_test(NAME ${TEST_NAME}_cpp11 COMMAND run_${TEST_NAME}_cpp11)

  add_executable(run_${TEST_NAME}_cpp17 ${TEST_NAME}.cpp)
  target_link_libraries(run_${TEST_NAME}_cpp17 PRIVATE unit_testing_cpp17)
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

add_library(unit_testing_cpp11 INTERFACE)
target_compile_features(unit_testing_cpp11 INTERFACE cxx_std_11)
target_link_libraries(unit_testing_cpp11 INTERFACE unit_testing)

add_library(unit_testing_cpp17 INTERFACE)
target_compile_features(unit_testing_cpp17 INTERFACE cxx_std_17)
target_link_libraries(unit_testing_cpp17 INTERFACE unit_testing)

add_cpp11_and_cpp17_test(dispatcher)
add_cpp11_and_cpp17_test(key_base)
add_cpp11_and_cpp17_test(key)
add_cpp11_and_cpp17_test(keyring)
add_cpp11_and_cpp17_test(order)
