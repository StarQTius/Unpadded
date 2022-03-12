FetchContent_MakeAvailable(Unity)

add_library(unpadded_ut INTERFACE)
target_compile_options(
  unpadded_ut
  INTERFACE
    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
    -Wall
    -Wextra
    -Werror>)
target_link_libraries(unpadded_ut INTERFACE unity::framework Unpadded)

add_executable(run_tuple_cpp11 tuple.cpp)
target_compile_features(run_tuple_cpp11 INTERFACE cxx_std_11)
target_link_libraries(run_tuple_cpp11 unpadded_ut)
add_test(NAME tuple_cpp11 COMMAND run_tuple_cpp11)

add_executable(run_tuple_view_cpp11 tuple_view.cpp)
target_compile_features(run_tuple_view_cpp11 INTERFACE cxx_std_11)
target_link_libraries(run_tuple_view_cpp11 unpadded_ut)
add_test(NAME tuple_view_cpp11 COMMAND run_tuple_view_cpp11)

add_executable(run_unaligned_data_cpp11 unaligned_data.cpp)
target_compile_features(run_unaligned_data_cpp11 INTERFACE cxx_std_11)
target_link_libraries(run_unaligned_data_cpp11 unpadded_ut)
add_test(NAME unaligned_data_cpp11 COMMAND run_unaligned_data_cpp11)

add_executable(run_tuple_cpp17 tuple.cpp)
target_compile_features(run_tuple_cpp17 INTERFACE cxx_std_17)
target_link_libraries(run_tuple_cpp17 unpadded_ut)
add_test(NAME tuple_cpp17 COMMAND run_tuple_cpp17)

add_executable(run_tuple_view_cpp17 tuple_view.cpp)
target_compile_features(run_tuple_view_cpp17 INTERFACE cxx_std_17)
target_link_libraries(run_tuple_view_cpp17 unpadded_ut)
add_test(NAME tuple_view_cpp17 COMMAND run_tuple_view_cpp17)

add_executable(run_unaligned_data_cpp17 unaligned_data.cpp)
target_compile_features(run_unaligned_data_cpp17 INTERFACE cxx_std_17)
target_link_libraries(run_unaligned_data_cpp17 unpadded_ut)
add_test(NAME unaligned_data_cpp17 COMMAND run_unaligned_data_cpp17)
