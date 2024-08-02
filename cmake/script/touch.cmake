cmake_minimum_required(VERSION 3.5)

include(ParseCliArguments)

parse_cli_arguments(POSITIONALS FILE)

cmake_path(REMOVE_FILENAME FILE OUTPUT_VARIABLE DIR)
cmake_path(NATIVE_PATH FILE FILE)
cmake_path(NATIVE_PATH DIR DIR)

file(MAKE_DIRECTORY ${DIR})
file(TOUCH ${FILE})
