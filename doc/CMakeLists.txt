configure_file(${CMAKE_CURRENT_SOURCE_DIR}/conf.py.in
               ${CMAKE_CURRENT_BINARY_DIR}/conf.py @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in
               ${CMAKE_CURRENT_BINARY_DIR}/Doxyfile @ONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/index.rst.in
               ${CMAKE_CURRENT_BINARY_DIR}/index.rst @ONLY)

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
