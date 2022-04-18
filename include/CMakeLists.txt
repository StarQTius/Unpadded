FetchContent_MakeAvailable(Boost)

set(UNPADDED_HEADERS
    buffered_dispatcher.hpp
    dispatcher.hpp
    flist.hpp
    key.hpp
    keyring.hpp
    action.hpp
    policy.hpp
    detail/any_function.hpp
    detail/def.hpp
    detail/type_traits/input_tuple.hpp
    detail/io/immediate_process.hpp
    detail/io/immediate_reader.hpp
    detail/io/immediate_writer.hpp
    detail/io/reader.hpp
    detail/io/writer.hpp
    detail/type_traits/flatten_tuple.hpp
    detail/serialized_message.hpp
    detail/type_traits/smallest.hpp
    detail/type_traits/conjunction.hpp
    detail/type_traits/remove_cv_ref.hpp
    detail/type_traits/detector.hpp
    detail/type_traits/signature.hpp
    detail/type_traits/is_byte_iterator.hpp
    detail/type_traits/is_keyring.hpp
    detail/type_traits/is_tuple.hpp
    detail/type_traits/require.hpp
    detail/type_traits/typelist.hpp
    detail/static_error.hpp
    detail/undef.hpp
    unevaluated.hpp
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
  include(Unpadded)

  foreach(HEADER IN LISTS UNPADDED_HEADERS)
    add_iwyu_target(upd/${HEADER})
  endforeach()
endif()
