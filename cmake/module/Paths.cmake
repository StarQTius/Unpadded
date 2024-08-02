cmake_path(APPEND MODULE_PATH ${PROJECT_SOURCE_DIR} cmake module)

cmake_path(APPEND IWYU_LINTER_SCRIPT ${PROJECT_SOURCE_DIR} cmake script
           run-include-what-you-use.cmake)
cmake_path(APPEND TOUCH_SCRIPT ${PROJECT_SOURCE_DIR} cmake script touch.cmake)
