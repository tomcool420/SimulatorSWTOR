function(add_static_library)
  cmake_parse_arguments(STATIC_LIB_ARGS "IS_TIDY" "" "EXTRA_SOURCE;TEST_EXTRA_SOURCE;SUBDIRECTORIES" ${ARGN})

  get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  set(target_name ${dirname})

  # Add all header, source and resources...
  file(GLOB DetailHeaders detail/*.h)
  file(GLOB DetailSources detail/*.cpp detail/*.c)

  file(GLOB Headers *.h)
  file(GLOB Sources *.cpp *.c)
  foreach(TMPSUB ${STATIC_LIB_ARGS_SUBDIRECTORIES})
    file(GLOB TMPHeaders ${TMPSUB}/*.h)
    list(APPEND Headers ${TMPHeaders})
    file(GLOB TMPSources ${TMPSUB}/*.cpp ${TMPSUB}/*.c)
    list(APPEND Sources ${TMPSources})
  endforeach()

  set(inputs ${Headers} ${Sources} ${DetailHeaders} ${DetailSources} ${Resources} ${STATIC_LIB_ARGS_EXTRA_SOURCE})

    add_library(${target_name} STATIC ${inputs})


  set(SIM_CURRENT_TARGET
      ${target_name}
      PARENT_SCOPE)
  set(SIM_CURRENT_TARGET ${target_name})

  source_group(Source ".hpp|.h|.cpp")
  source_group(detail REGULAR_EXPRESSION "detail\*")

  set_target_properties(${target_name} PROPERTIES FOLDER ${SIM_PROJECT})

  if(TargetOSX)
    set_target_properties(${ProjectName} PROPERTIES XCODE_ATTRIBUTE_GCC_GENERATE_DEBUGGING_SYMBOLS "YES")
  endif()

  if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/test)
    add_test_project(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/test NAME "${dirname}Test" EXTRA_SOURCE
                     ${STATIC_LIB_ARGS_TEST_EXTRA_SOURCE})
    set(SIM_CURRENT_TEST
        ${SIM_CURRENT_TEST}
        PARENT_SCOPE)
    set(SIM_CURRENT_TEST ${SIM_CURRENT_TEST})
  endif()
endfunction(add_static_library)
