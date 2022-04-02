add_custom_target(
  Doxygen-xml
  COMMAND doxygen
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

add_custom_target(
  doc
  COMMAND sphinx-build -b html ${CMAKE_CURRENT_SOURCE_DIR}
          ${CMAKE_CURRENT_BINARY_DIR}
  DEPENDS Doxygen-xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
