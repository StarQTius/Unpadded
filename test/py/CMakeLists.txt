FetchContent_MakeAvailable(pybind11)

find_package(Python3 COMPONENTS Interpreter Development)
set(PYTEST_COMMAND ${Python3_EXECUTABLE} -B -m pytest --color=yes -p
                   no:cacheprovider)

pybind11_add_module(module module.cpp)
target_link_libraries(module PRIVATE unity::framework ${PROJECT_NAME})
set_target_properties(module PROPERTIES INTERPROCEDURAL_OPTIMIZATION ON
                                        CXX_VISIBILITY_PRESET hidden)
target_compile_features(module PUBLIC cxx_std_17)

add_test(
  NAME python_test
  COMMAND ${PYTEST_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/test.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

set_tests_properties(python_test PROPERTIES LABELS check)
