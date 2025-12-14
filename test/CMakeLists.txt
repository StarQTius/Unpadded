FetchContent_MakeAvailable(Catch2)
FetchContent_MakeAvailable(FakeIt)

include(Catch)
include(CTest)
include(Unpadded)

file(
  GLOB_RECURSE HEADERS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  CONFIGURE_DEPENDS *.hpp)

add_executable(upd_unitary
  algebra.cpp
  record.cpp
  tuple.cpp)
target_link_libraries(upd_unitary PRIVATE
  Unpadded::UnpaddedDebug
  Catch2::Catch2WithMain
  FakeIt::FakeIt-catch
)
catch_discover_tests(upd_unitary PROPERTIES LABELS unitary)
upd_target_lintables(iwyu upd_unitary PRIVATE ${HEADERS})
upd_target_lintables(clang_format upd_unitary PRIVATE ${HEADERS})
upd_target_lintables(clang_tidy upd_unitary)

add_custom_target(upd_examples)
add_dependencies(upd_examples Unpadded::Dynamixel2)
upd_register_run_and_compare_tests(upd_examples examples)
