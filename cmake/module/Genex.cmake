include(NormalizePath)

# `properties_genexpr(<OUTPUT> <TARGET> <PROPERTIES...>)`
#
# Build a generator expression that expresses `(<property name from PROPERTIES>
# <corresponding property value of TARGET>)...` and store it in `OUTPUT`
function(properties_genex OUTPUT TARGET)
  foreach(PROP IN LISTS ARGN)
    list(APPEND RETVAL ${PROP} $<TARGET_PROPERTY:${TARGET},${PROP}>)
  endforeach()

  set(${OUTPUT}
      ${RETVAL}
      PARENT_SCOPE)
endfunction()

# `is_source_of_target_genex(<OUTPUT> <TARGET> <FILE>)`
#
# Build a generator expression that checks whether `FILE` is a source of
# `TARGET` and store it in `OUTPUT`
function(is_source_of_target_genex OUTPUT TARGET FILE)
  normalize_path(FILE)

  set(SOURCES_GENEX $<TARGET_PROPERTY:${TARGET},SOURCES>)
  set(SOURCE_DIR_GENEX $<TARGET_PROPERTY:${TARGET},SOURCE_DIR>)
  set(ABSOLUTE_SOURCES_GENEX
      $<PATH:ABSOLUTE_PATH,NORMALIZE,${SOURCES_GENEX},${SOURCE_DIR_GENEX}>)
  set(FIND_FILE_IN_SOURCES_GENEX $<LIST:FIND,${ABSOLUTE_SOURCES_GENEX},${FILE}>)

  set(${OUTPUT}
      "$<NOT:$<EQUAL:${FIND_FILE_IN_SOURCES_GENEX},-1>>"
      PARENT_SCOPE)
endfunction()
