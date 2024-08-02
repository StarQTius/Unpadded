include(RequireDefined)

# `parse_cli_function(POSITIONALS <POSITIONAL_ARGUMENT_NAMES...> KEYWORDS
# <KEYWORD_ARGUMENT_NAMES...> OPTIONS <OPTIONAL_ARGUMENT_NAMES...>)`
#
# Parse the arguments passed from command line and set variables whose names
# belong to `POSITIONAL_ARGUMENT_NAMES`, `KEYWORD_ARGUMENT_NAMES` and
# `OPTIONAL_ARGUMENT_NAMES`
#
# The commandline invokation must comply with the following format: `<cmake
# command> <cmake command flags> -P <invoked script> -- <positional arguments>
# (<keyword from KEYWORD_ARGUMENT_NAMES> <value>)... <option from
# OPTIONAL_ARGUMENT_NAMES>...`. Positional arguments are mandatory, keyword and
# optional arguments are not.
function(parse_cli_arguments)
  foreach(I RANGE ${CMAKE_ARGC})
    list(APPEND CLI_ARGV ${CMAKE_ARGV${I}})
  endforeach()

  cmake_parse_arguments(PARSE_ARGV 0 "PARSED" "" ""
                        "POSITIONALS;KEYWORDS;OPTIONS")
  cmake_parse_arguments("ARGV" "${PARSED_OPTIONS}" "" "--;${PARSED_KEYWORDS}"
                        ${CLI_ARGV})

  set(POSITIONALS ${ARGV_--})
  foreach(KEYWORD VALUE IN ZIP_LISTS PARSED_POSITIONALS POSITIONALS)
    require_defined_with_message(KEYWORD "Too many positional arguments")
    require_defined_with_message(
      VALUE "`${KEYWORD}` positional argument must be provided")
    set(${KEYWORD}
        ${VALUE}
        PARENT_SCOPE)
  endforeach()

  foreach(KEYWORD IN LISTS PARSED_KEYWORDS)
    set(${KEYWORD}
        ${ARGV_${KEYWORD}}
        PARENT_SCOPE)
  endforeach()

  foreach(OPTION IN LISTS PARSED_OPTIONS)
    set(${OPTION}
        ${ARGV_${OPTION}}
        PARENT_SCOPE)
  endforeach()
endfunction()
