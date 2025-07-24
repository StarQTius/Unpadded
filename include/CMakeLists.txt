add_library(upd_Unpadded INTERFACE)
target_include_directories(upd_Unpadded INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_compile_features(upd_Unpadded INTERFACE cxx_std_23)

add_library(upd_UnpaddedDebug INTERFACE)
target_link_libraries(upd_UnpaddedDebug INTERFACE upd_Unpadded)
target_compile_definitions(upd_UnpaddedDebug INTERFACE UPD_DEBUG)

if(PROJECT_IS_TOP_LEVEL)
  include(Unpadded)

  add_iwyu_target(iwyu upd_Unpadded)

  file(
    GLOB_RECURSE HEADERS
    RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}
    CONFIGURE_DEPENDS *.hpp)
  target_lintables(iwyu ${HEADERS})
endif()
