include(GoogleTest)

function(add_test)
  cmake_parse_arguments(TESTS "" "DIRECTORY;NAME" "EXTRA_SOURCE;PROPERTIES" ${ARGN})
  if(NOT TESTS_NAME)
    get_filename_component(dirname ${CMAKE_CURRENT_SOURCE_DIR} NAME)
  else()
    set(dirname ${TESTS_NAME})
  endif()

  set(target_name ${dirname})

  if(NOT TESTS_DIRECTORY)
    file(GLOB Headers *.h)
    file(GLOB Sources *.cpp)
    file(GLOB Resources *.avdl)
    set(SIM_CURRENT_TARGET
        ${target_name}
        PARENT_SCOPE)
  else()
    file(GLOB Headers ${TESTS_DIRECTORY}/*.h)
    file(GLOB Sources ${TESTS_DIRECTORY}/*.cpp)
    file(GLOB Resources ${TESTS_DIRECTORY}/*.avdl)
    set(SIM_CURRENT_TEST
        ${target_name}
        PARENT_SCOPE)
  endif()
  add_executable(
    ${target_name}
    ${Headers}
    ${Sources}
    ${Resources}
    ${TESTS_EXTRA_SOURCE})
  # This is the tests project file that will be created

  # signed/unsigned comparison gtest.h

  target_link_libraries(${target_name} PRIVATE CONAN_PKG::gtest)

    if("${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION}.${CMAKE_PATCH_VERSION}" VERSION_LESS "3.10.3")
      set(timeout_name TIMEOUT)
    else()
      set(timeout_name DISCOVERY_TIMEOUT)
    endif()
    gtest_discover_tests(${target_name} TEST_PREFIX ${target_name}: ${timeout_name} 60)
  source_group(Source ".hpp|.h|.cpp")

endfunction(add_test)
