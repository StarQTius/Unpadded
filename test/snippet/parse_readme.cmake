file(READ ${README_PATH} README)
string(REPLACE ";" "@" README "${README}")
string(REGEX MATCHALL "```cpp[^`]*```" SNIPPETS "${README}")
set(SNIPPET_FILENAMES shared.hpp caller.cpp callee.cpp)

foreach(SNIPPET IN ZIP_LISTS SNIPPETS SNIPPET_FILENAMES)
  string(REPLACE "```cpp" "" SNIPPET_0 ${SNIPPET_0})
  string(REPLACE "```" "" SNIPPET_0 ${SNIPPET_0})
  string(REPLACE "@" ";" SNIPPET_0 "${SNIPPET_0}")
  file(CONFIGURE OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${SNIPPET_1} CONTENT
       "${SNIPPET_0}")
endforeach()
