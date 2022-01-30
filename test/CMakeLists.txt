set(Unpadded_TEST_SOURCES
  main.cpp
  tuple_view.cpp
  tuple.cpp
  unaligned_data.cpp)

add_executable(run_ut11 ${Unpadded_TEST_SOURCES})
set_target_properties(run_ut11 PROPERTIES CXX_STANDARD 11)
target_compile_options(run_ut11 PRIVATE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wall -Werror>
  $<$<CXX_COMPILER_ID:MSVC>: /W4 /WX>)

target_link_libraries(run_ut11 PRIVATE unity::framework Unpadded)
add_test(ut11 COMMAND run_ut11)

add_executable(run_ut17 ${Unpadded_TEST_SOURCES})
target_include_directories(run_ut17 PRIVATE ${PROJECT_SOURCE_DIR}/include)
set_target_properties(run_ut17 PROPERTIES CXX_STANDARD 17)
target_compile_options(run_ut17 PRIVATE
  $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>: -Wall -Werror>
  $<$<CXX_COMPILER_ID:MSVC>: /W4 /WX>)

target_link_libraries(run_ut17 PRIVATE unity::framework Unpadded)
add_test(ut17 COMMAND run_ut17)
