find_package(Python3 COMPONENTS Interpreter Development)
set(PYTEST_COMMAND ${Python3_EXECUTABLE} -B -m pytest --color=yes -s -p
                   no:cacheprovider)

add_custom_target(
  check_python
  COMMAND ctest -L ^check_python$$ --output-on-failure
  DEPENDS _details)

add_test(
  NAME python_test
  COMMAND ${PYTEST_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/test.py
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}/py)

set_tests_properties(
  python_test PROPERTIES LABELS check_python ENVIRONMENT
                         "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}")
