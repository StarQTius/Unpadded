add_library(Unpadded INTERFACE)
target_include_directories(Unpadded INTERFACE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(Unpadded INTERFACE Boost::mp11 Boost::type_traits)
