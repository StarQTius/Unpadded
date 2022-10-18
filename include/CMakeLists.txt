file(
  GLOB_RECURSE HEADERS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  CONFIGURE_DEPENDS *.hpp)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
  ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIRS}>)
if(${PROJECT_NAME}_IS_TOP_LEVEL)
  include(Unpadded)

  foreach(HEADER IN LISTS HEADERS)
    add_iwyu_target(${HEADER} ${PROJECT_NAME})
  endforeach()
endif()
