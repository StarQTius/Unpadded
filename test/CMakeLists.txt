FetchContent_MakeAvailable(Unity)

set(UNPADDED_TEST_SOURCES main.cpp tuple_view.cpp tuple.cpp unaligned_data.cpp)

add_library(unpadded_ut INTERFACE)
target_compile_options(
  unpadded_ut
  INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall
    -Wextra
    -Werror>)
target_link_libraries(unpadded_ut INTERFACE unity::framework Unpadded)

add_executable(run_ut11 ${UNPADDED_TEST_SOURCES})
target_compile_features(unpadded_ut INTERFACE cxx_std_11)
target_link_libraries(run_ut11 unpadded_ut)
add_test(NAME ut11 COMMAND run_ut11)

add_executable(run_ut17 ${UNPADDED_TEST_SOURCES})
target_compile_features(unpadded_ut INTERFACE cxx_std_17)
target_link_libraries(run_ut17 unpadded_ut)
add_test(NAME ut17 COMMAND run_ut17)
