set(HEADERS
    action.hpp
    buffered_dispatcher.hpp
    dispatcher.hpp
    detail/def.hpp
    detail/endianess.hpp
    detail/io/immediate_process.hpp
    detail/io/immediate_reader.hpp
    detail/io/immediate_writer.hpp
    detail/serialized_message.hpp
    detail/signed_representation.hpp
    detail/static_error.hpp
    detail/type_traits/conjunction.hpp
    detail/type_traits/detector.hpp
    detail/type_traits/flatten_tuple.hpp
    detail/type_traits/index_sequence.hpp
    detail/type_traits/input_tuple.hpp
    detail/type_traits/is_byte_iterator.hpp
    detail/type_traits/is_key.hpp
    detail/type_traits/is_keyring.hpp
    detail/type_traits/is_tuple.hpp
    detail/type_traits/remove_cv_ref.hpp
    detail/type_traits/require.hpp
    detail/type_traits/smallest.hpp
    detail/type_traits/signature.hpp
    detail/type_traits/typelist.hpp
    detail/undef.hpp
    format.hpp
    key.hpp
    keyring.hpp
    policy.hpp
    python.hpp
    serialization.hpp
    tuple.hpp
    type.hpp
    typelist.hpp
    unevaluated.hpp)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(
  ${PROJECT_NAME} INTERFACE $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
                            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIRS}>)
if(${PROJECT_NAME}_IS_TOP_LEVEL)
  include(Unpadded)

  foreach(HEADER IN LISTS HEADERS)
    add_iwyu_target(upd/${HEADER} ${PROJECT_NAME})
  endforeach()
endif()
