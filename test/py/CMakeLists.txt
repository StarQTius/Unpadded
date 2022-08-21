find_package(Python3 COMPONENTS Interpreter Development)
set(PYTEST_COMMAND ${Python3_EXECUTABLE} -B -m pytest --color=yes -p
                   no:cacheprovider)

add_test(
  NAME python_test
  COMMAND ${PYTEST_COMMAND} ${CMAKE_CURRENT_SOURCE_DIR}/test.py
  WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/py)

set_tests_properties(
  python_test PROPERTIES LABELS check ENVIRONMENT
                         "PYTHONPATH=${CMAKE_CURRENT_BINARY_DIR}")
