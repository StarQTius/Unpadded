FetchContent_MakeAvailable(Boost)

set(UNPADDED_HEADERS
    array_wrapper.hpp
    format.hpp
    serialization.hpp
    tuple.hpp
    type.hpp
    unaligned_data.hpp
    detail/def.hpp
    detail/endianess.hpp
    detail/sfinae.hpp
    detail/signature.hpp
    detail/signed_representation.hpp
    detail/undef.hpp
    detail/value_h.hpp)

add_library(Unpadded INTERFACE)
target_include_directories(Unpadded INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(Unpadded INTERFACE Boost::mp11 Boost::type_traits)
if(Unpadded_IS_TOP_LEVEL AND DEFINED CMAKE_CXX_INCLUDE_WHAT_YOU_USE)
  include(UnpaddedIWYU)

  foreach(HEADER IN LISTS UNPADDED_HEADERS)
    add_iwyu_target(upd/${HEADER})
  endforeach()
endif()
