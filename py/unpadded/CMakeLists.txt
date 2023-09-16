FetchContent_MakeAvailable(pybind11)

pybind11_add_module(_details _details.cpp)
target_link_libraries(_details PRIVATE ${PROJECT_NAME})
file(COPY __init__.py __main__.py client.py hook.py
     DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
