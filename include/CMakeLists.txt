FetchContent_MakeAvailable(expected Unpadded)

set(K2O_HEADERS
    buffered_dispatcher.hpp
    cpp11.hpp
    dispatcher.hpp
    key_base.hpp
    key.hpp
    keyring.hpp
    order.hpp
    policy.hpp
    detail/any_function.hpp
    detail/def.hpp
    detail/input_tuple.hpp
    detail/io.hpp
    detail/normalize_to_tuple.hpp
    detail/sfinae.hpp
    detail/signature.hpp
    detail/typelist.hpp
    detail/undef.hpp
    detail/value_h.hpp)

add_library(${PROJECT_NAME} INTERFACE)
target_include_directories(${PROJECT_NAME}
                           INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(${PROJECT_NAME} INTERFACE expected Unpadded)

if(DEFINED CMAKE_CXX_INCLUDE_WHAT_YOU_USE)
  foreach(HEADER IN LISTS K2O_HEADERS)
    add_iwyu_target(k2o/${HEADER} ${PROJECT_NAME})
  endforeach()
endif()
