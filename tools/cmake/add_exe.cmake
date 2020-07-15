function(add_exe)
  cmake_parse_arguments(ARGS "" "" "EXTRA_SOURCE" ${ARGN})

  get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  set(target_name ${dirname})
  set(SIM_CURRENT_TARGET
      ${target_name}
      PARENT_SCOPE)
  set(SIM_CURRENT_TARGET ${target_name})

  file(GLOB Headers *.h)
  file(GLOB Sources *.cpp *.c)

    add_executable(${target_name} ${Headers}
    ${Sources}
    ${Resources}
    ${ARGS_EXTRA_SOURCE})

  source_group("Source" FILES ".hpp|.h|.cpp")

endfunction(add_exe)
