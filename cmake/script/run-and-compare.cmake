cmake_minimum_required(VERSION 3.10)

include(UpdParseCliArguments)

find_program(DIFF_COMMAND diff
             DOC "'diff' Unix command" REQUIRED)

upd_parse_cli_arguments(
  POSITIONALS
  "EXECUTABLE"
  "REFERENCE_OUTPUT"
)

execute_process(
  COMMAND ${EXECUTABLE}
  COMMAND ${DIFF_COMMAND} --color=always ${REFERENCE_OUTPUT} -
  RESULT_VARIABLE COMMAND_RESULT)

cmake_language(EXIT ${COMMAND_RESULT})
