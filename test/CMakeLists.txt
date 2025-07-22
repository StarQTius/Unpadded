FetchContent_MakeAvailable(Catch2)
FetchContent_MakeAvailable(FakeIt)

include(Catch)
include(CTest)
include(Unpadded)

set(UPD_CTEST_FLAGS --output-on-failure)

add_executable(unitary
  named_tuple_v2.cpp)
target_link_libraries(unitary PRIVATE
  Unpadded::UnpaddedDebug
  Catch2::Catch2WithMain
  FakeIt::FakeIt-catch
)
catch_discover_tests(unitary PROPERTIES LABELS unitary)

add_custom_target(check COMMAND ctest ${UPD_CTEST_FLAGS} -L unitary DEPENDS unitary)
add_custom_target(check_full COMMAND ctest ${UPD_CTEST_FLAGS} -L examples unitary DEPENDS examples unitary)
add_custom_target(check_unitary COMMAND ctest ${UPD_CTEST_FLAGS} -L unitary DEPENDS unitary)
