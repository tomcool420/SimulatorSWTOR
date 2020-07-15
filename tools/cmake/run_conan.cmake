if(V3D_REMOVE_CONAN_LOCKS)
  execute_process(COMMAND ${PYTHON_EXECUTABLE} -m conans.conan remove --locks)
endif()


set(CONAN_FLAGS "")

string(REPLACE "." ";" SPLIT_COMPILER_VERSION ${CMAKE_CXX_COMPILER_VERSION})

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
  list(GET SPLIT_COMPILER_VERSION 0 GCC_VERSION)
  list(
    APPEND
    CONAN_FLAGS
    -s
    compiler=gcc
    -s
    compiler.libcxx=libstdc++11
    -s
    compiler.version=${GCC_VERSION})
endif()

if(MSVC)
  if("${CMAKE_CXX_FLAGS}" MATCHES "/MDd")
    list(APPEND CONAN_FLAGS -s compiler.runtime=MDd)
  elseif("${CMAKE_CXX_FLAGS}" MATCHES "/MD")
    list(APPEND CONAN_FLAGS -s compiler.runtime=MD)
  elseif("${CMAKE_CXX_FLAGS}" MATCHES "/MTd")
    list(APPEND CONAN_FLAGS -s compiler.runtime=MTd)
  elseif("${CMAKE_CXX_FLAGS}" MATCHES "/MT")
    list(APPEND CONAN_FLAGS -s compiler.runtime=MT)
  endif()
  list(APPEND CONAN_FLAGS -s "compiler=Visual Studio")
  message(STATUS "version ${CMAKE_CXX_COMPILER_VERSION}")
  if(CMAKE_CXX_COMPILER_VERSION MATCHES "19\.2.*")
    set(VS_VERSION 16)
  elseif(CMAKE_CXX_COMPILER_VERSION MATCHES "19\.1.*")
    set(VS_VERSION 15)
  else()
    set(VS_VERSION 14)
  endif()
  list(APPEND CONAN_FLAGS -s compiler.version=${VS_VERSION})
endif()


if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang")
  list(GET SPLIT_COMPILER_VERSION 0 CLANG_VERSION)
  list(APPEND CONAN_FLAGS -s compiler=clang -s compiler.version=${CLANG_VERSION})
  list(APPEND CONAN_FLAGS -s compiler.libcxx=libstdc++11)
  string(REPLACE "." ";" verSplit ${CMAKE_CXX_COMPILER_VERSION})
  list(GET verSplit 0 major)
  list(GET verSplit 1 minor)
  set(compilerVer "${major}.${minor}")
  list(APPEND CONAN_FLAGS -s compiler.version=${compilerVer})
endif()

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "AppleClang")
  message(STATUS "APPLECLANG CONAN FLAGS")
  list(APPEND CONAN_FLAGS -s compiler=apple-clang -s compiler.libcxx=libc++)
  string(REPLACE "." ";" verSplit ${CMAKE_CXX_COMPILER_VERSION})
  list(GET verSplit 0 major)
  list(GET verSplit 1 minor)
  set(compilerVer "${major}.${minor}")
  list(APPEND CONAN_FLAGS -s compiler.version=${compilerVer})
  list(APPEND CONAN_FLAGS -s os.version=${CMAKE_OSX_DEPLOYMENT_TARGET})
endif()

message(STATUS "CONAN: Copying libraries to bin/")
if("${CMAKE_BUILD_TYPE}" STREQUAL "Debug")
  list(APPEND CONAN_FLAGS -s build_type=Debug)
else()
  list(APPEND CONAN_FLAGS -s build_type=Release)
  if(NOT "${CMAKE_BUILD_TYPE}" STREQUAL "RelWithDebInfo")
    set(CMAKE_BUILD_TYPE Release)
  endif()
endif()
if(TargetOSX)
  execute_process(COMMAND xcrun --show-sdk-path OUTPUT_VARIABLE skdpath)
  set(ENV{MACOSX_DEPLOYMENT_TARGET} ${CMAKE_OSX_DEPLOYMENT_TARGET})
  set(ENV{SDKROOT} ${sdkpath})
endif()

set(conan_command ${PYTHON_EXECUTABLE} -m conans.conan install ${PROJECT_SOURCE_DIR} repo@velo3d/stable -g cmake -if
                  ${CMAKE_BINARY_DIR} --build missing ${CONAN_FLAGS})
message(STATUS "Running ${conan_command}")

execute_process(
  COMMAND ${conan_command}
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  RESULT_VARIABLE retcode)
if(NOT "${retcode}" STREQUAL "0")
  message(FATAL_ERROR "Fatal error running conan")
endif()
