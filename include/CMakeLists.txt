file(
  GLOB_RECURSE HEADERS
  RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
  CONFIGURE_DEPENDS *.hpp)
list(REMOVE_ITEM HEADERS upd/python.hpp)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
  ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIRS}>)
target_compile_features(${PROJECT_NAME} INTERFACE cxx_std_23)

if(${PROJECT_NAME}_PLATFORM_ENDIANESS)
  target_compile_definitions(
    ${PROJECT_NAME}
    INTERFACE UPD_PLATFORM_ENDIANESS=${${PROJECT_NAME}_PLATFORM_ENDIANESS})
endif()

if(${PROJECT_NAME}_PLATFORM_SIGNED_MODE)
  target_compile_definitions(
    ${PROJECT_NAME}
    INTERFACE UPD_PLATFORM_SIGNED_MODE=${${PROJECT_NAME}_PLATFORM_SIGNED_MODE})
endif()

if(${PROJECT_NAME}_IS_TOP_LEVEL)
  include(Unpadded)

  add_iwyu_target(iwyu ${PROJECT_NAME})
  target_lintables(iwyu ${HEADERS})
endif()
