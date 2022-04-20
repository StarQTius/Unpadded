FetchContent_MakeAvailable(Boost)

set(UNPADDED_HEADERS
    action.hpp
    buffered_dispatcher.hpp
    dispatcher.hpp
    detail/def.hpp
    detail/endianess.hpp
    detail/io/immediate_process.hpp
    detail/io/immediate_reader.hpp
    detail/io/immediate_writer.hpp
    detail/io/reader.hpp
    detail/io/writer.hpp
    detail/serialized_message.hpp
    detail/signed_representation.hpp
    detail/static_error.hpp
    detail/type_traits/conjunction.hpp
    detail/type_traits/detector.hpp
    detail/type_traits/flatten_tuple.hpp
    detail/type_traits/input_tuple.hpp
    detail/type_traits/is_byte_iterator.hpp
    detail/type_traits/is_keyring.hpp
    detail/type_traits/is_tuple.hpp
    detail/type_traits/remove_cv_ref.hpp
    detail/type_traits/require.hpp
    detail/type_traits/smallest.hpp
    detail/type_traits/signature.hpp
    detail/type_traits/typelist.hpp
    detail/undef.hpp
    flist.hpp
    format.hpp
    key.hpp
    keyring.hpp
    policy.hpp
    serialization.hpp
    tuple.hpp
    type.hpp
    unaligned_data.hpp
    unevaluated.hpp)

add_library(Unpadded INTERFACE)
target_include_directories(Unpadded INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(Unpadded INTERFACE Boost::mp11)
if(Unpadded_IS_TOP_LEVEL AND DEFINED CMAKE_CXX_INCLUDE_WHAT_YOU_USE)
  include(Unpadded)

  foreach(HEADER IN LISTS UNPADDED_HEADERS)
    message(${HEADER})
    add_iwyu_target(upd/${HEADER} Unpadded)
  endforeach()
endif()
