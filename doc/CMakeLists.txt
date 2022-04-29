set(TEMPLATE_FILES conf.py Doxyfile index.rst shared.rst caller.rst callee.rst)

foreach(TEMPLATE_FILE IN LISTS TEMPLATE_FILES)
  configure_file(${CMAKE_CURRENT_SOURCE_DIR}/${TEMPLATE_FILE}.in
                 ${CMAKE_CURRENT_BINARY_DIR}/${TEMPLATE_FILE} @ONLY)
endforeach()

add_custom_target(
  Doxygen-xml
  COMMAND doxygen
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(
  doc
  COMMAND sphinx-build -b html ${CMAKE_CURRENT_BINARY_DIR}
          ${CMAKE_BINARY_DIR}/Unpadded
  DEPENDS Doxygen-xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
