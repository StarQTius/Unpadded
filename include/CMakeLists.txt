FetchContent_MakeAvailable(expected Unpadded)

set(K2O_HEADERS
    buffered_dispatcher.hpp
    dispatcher.hpp
    flist.hpp
    key.hpp
    keyring.hpp
    order.hpp
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
    unevaluated.hpp)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} INTERFACE expected Unpadded)

if(DEFINED CMAKE_CXX_INCLUDE_WHAT_YOU_USE)
  foreach(HEADER IN LISTS K2O_HEADERS)
    add_iwyu_target(k2o/${HEADER} ${PROJECT_NAME})
  endforeach()
endif()
