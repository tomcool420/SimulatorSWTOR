cmake_minimum_required(VERSION 3.11.0)

set(CMAKE_DISABLE_SOURCE_CHANGES ON)
set(CMAKE_DISABLE_IN_SOURCE_BUILD ON)

set_property(GLOBAL PROPERTY USE_FOLDERS On)
cmake_policy(SET CMP0071 NEW)
cmake_policy(SET CMP0068 NEW)

project(SimulatorSWTOR)
set(CMAKE_MODULE_PATH "${PROJECT_SOURCE_DIR}/tools/cmake/" ${CMAKE_MODULE_PATH})


set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CONFIGURATION_TYPES Debug Release RelWithDebInfo)
set(CMAKE_OSX_DEPLOYMENT_TARGET 10.14)
option(CALL_PIP "Install pip dependencies" ON)
option(CALL_CONAN "Install conan dependencies" ON)
option(LOCAL_CONAN_HOME, "local directory for conan" OFF)
option(ENABLE_COVERALLS,"Enable Coveralls" OFF)
if(${LOCAL_CONAN_HOME})
set(ENV{CONAN_USER_HOME} ${CMAKE_SOURCE_DIR}/.conan)
endif()

find_package(PythonInterp 3 REQUIRED)
execute_process(COMMAND ${PYTHON_EXECUTABLE} -E -m venv ${CMAKE_BINARY_DIR}/venv/
                RESULT_VARIABLE venv_retcode)
if(NOT "${venv_retcode}" STREQUAL "0")
  message(FATAL_ERROR "Fatal error running venv")
endif()

if(${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
  set(PYTHON_EXECUTABLE ${CMAKE_BINARY_DIR}/venv/Scripts/python.exe)
else()
  set(PYTHON_EXECUTABLE ${CMAKE_BINARY_DIR}/venv/bin/python)
endif()

if(ENABLE_COVERALLS)
  include(Coveralls)
  coveralls_turn_on_coverage()


endif()


if(CALL_PIP STREQUAL "ON")
  file(TIMESTAMP ${CMAKE_SOURCE_DIR}/tools/requirements.txt REQUIREMENTS_TIMESTAMP_T "%Y%j%H%M%S")
  if(NOT DEFINED REQUIREMENTS_TIMESTAMP)
    message(STATUS "Python requirements.txt has not been run yet")
  elseif(${REQUIREMENTS_TIMESTAMP_T} GREATER ${REQUIREMENTS_TIMESTAMP})
    message(STATUS "Python requirements.txt has been updated")
  else()
    message(STATUS "Python requirements.txt does not need to be reparsed")
    set(CALL_PIP "OFF")
  endif()
endif()

if(CALL_PIP STREQUAL "ON")
  set(REQUIREMENTS_TIMESTAMP ${REQUIREMENTS_TIMESTAMP_T} CACHE INTERNAL "Date Modified on requirements.txt")
  message(STATUS "Running pip with ${PYTHON_EXECUTABLE}")
  execute_process(COMMAND ${PYTHON_EXECUTABLE} -m pip install --upgrade -q -r
                          ${CMAKE_SOURCE_DIR}/tools/requirements.txt
                          RESULT_VARIABLE pip_retcode)
  if(NOT "${pip_retcode}" STREQUAL "0")
    message(FATAL_ERROR "Fatal error running pip")
  endif()
endif()

set(conanfile_path ${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)

if(CALL_CONAN)
  if(NOT EXISTS "${conanfile_path}")
    message(STATUS "No Conan file found. Will run conan.")
    set(run_conan "ON")
  elseif(NOT ("${LAST_BUILD_TYPE}" STREQUAL "${CMAKE_BUILD_TYPE}"))
    message(STATUS "Build type changed. Will run conan.")
    set(run_conan "ON")
  else()
    file(TIMESTAMP ${CMAKE_SOURCE_DIR}/conanfile.txt input_conan_timestamp "%Y%j%H%M%S")
    file(TIMESTAMP ${conanfile_path} output_conan_timestamp "%Y%j%H%M%S")
    if(${input_conan_timestamp} GREATER ${output_conan_timestamp})
      message(STATUS "The conanfile has been updated. Will re run.")
      set(run_conan "ON")
    else()
      message(STATUS "Conan file is unchanged.")
      set(run_conan "OFF")
    endif()
  endif()
else()
  set(run_conan "OFF")
endif()

if(${run_conan})
  set(LAST_BUILD_TYPE ${CMAKE_BUILD_TYPE} CACHE INTERNAL "Build type last time cmake was run")
  include(run_conan)
endif()

include(${conanfile_path})
conan_basic_setup(TARGETS)
if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Qunused-arguments -Wno-missing-braces -Wno-conversion -Wunused-parameter -Wunused-variable -Wunused -Wshadow -Wno-deprecated")
endif()
set(SIM_PROJECT SimulatorSWTOR)

include(add_static_library)
include(add_exe)
include(add_test)

add_subdirectory(Simulator)