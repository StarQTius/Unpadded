FetchContent_MakeAvailable(Unity)
FetchContent_MakeAvailable(Catch2)
FetchContent_MakeAvailable(FakeIt)

include(Catch)

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

function(add_cpp11_and_cpp17_static_test TEST_NAME)
  add_executable(run_static_${TEST_NAME}_cpp11 ${TEST_NAME}.cpp)
  set_target_properties(run_static_${TEST_NAME}_cpp11 PROPERTIES CXX_STANDARD
                                                                 11)
  target_link_libraries(run_static_${TEST_NAME}_cpp11 PRIVATE unit_testing)
  add_test(NAME static_${TEST_NAME}_cpp11 COMMAND run_static_${TEST_NAME}_cpp11)
  set_tests_properties(static_${TEST_NAME}_cpp11 PROPERTIES LABELS static_check)

  add_executable(run_static_${TEST_NAME}_cpp17 ${TEST_NAME}.cpp)
  set_target_properties(run_static_${TEST_NAME}_cpp17 PROPERTIES CXX_STANDARD
                                                                 17)
  target_link_libraries(run_static_${TEST_NAME}_cpp17 PRIVATE unit_testing)
  add_test(NAME static_${TEST_NAME}_cpp17 COMMAND run_static_${TEST_NAME}_cpp17)
  set_tests_properties(static_${TEST_NAME}_cpp17 PROPERTIES LABELS static_check)

  add_dependencies(static_check run_static_${TEST_NAME}_cpp11
                   run_static_${TEST_NAME}_cpp17)
endfunction()

add_library(unit_testing INTERFACE)
target_compile_options(
  unit_testing
  INTERFACE -Wall -Werror $<$<CXX_COMPILER_ID:GNU>: -fdiagnostics-color=always>
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
            -fcolor-diagnostics>)
target_link_libraries(unit_testing INTERFACE unity::framework ${PROJECT_NAME})

add_custom_target(static_check COMMAND ctest -L static_check
                                       --output-on-failure)
add_custom_target(
  check
  COMMAND ctest -L check --output-on-failure
  DEPENDS static_check)

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
add_cpp11_and_cpp17_static_test(static)

# Revamp tests

function(add_revamp_test TEST_NAME TEST_FILE)
  add_executable(${TEST_NAME} ${TEST_FILE})
  target_link_libraries(${TEST_NAME} PRIVATE Unpadded Catch2::Catch2WithMain
                                             FakeIt::FakeIt-catch)
  set_target_properties(${TEST_NAME} PROPERTIES CXX_STANDARD 17)
  add_dependencies(check_revamp ${TEST_NAME})
  catch_discover_tests(${TEST_NAME} PROPERTIES LABELS check_revamp)
  target_compile_options(
    ${TEST_NAME}
    PRIVATE -Wall -Werror $<$<CXX_COMPILER_ID:GNU>: -fdiagnostics-color=always>
            $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:
            -fcolor-diagnostics>)
endfunction()

add_custom_target(check_revamp COMMAND ctest -L check_revamp
                                       --output-on-failure)

add_revamp_test(basic_tuple basic_tuple.cpp)
add_revamp_test(basic_ibytestream basic_ibytestream.cpp)
add_revamp_test(detail_variadic_clean detail/variadic/clean.cpp)
add_revamp_test(detail_variadic_clip detail/variadic/clip.cpp)
