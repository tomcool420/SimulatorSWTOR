add_subdirectory(libSimulator)
add_subdirectory(Classes)
<<<<<<< Updated upstream
add_subdirectory(Simulator)
=======
add_subdirectory(Simulator)
add_subdirectory(Rotations)

if(ENABLE_COVERALLS)
MESSAGE("Enabling Coveralls")
set (EXCLUDE_DIR "/test/")
file (GLOB_RECURSE SOURCE_FILES RELATIVE ${CMAKE_SOURCE_DIR} LIST_DIRECTORIES false "*.cpp" "*.h" )
foreach (TMP_PATH ${SOURCE_FILES})
    string (FIND ${TMP_PATH} ${EXCLUDE_DIR} EXCLUDE_DIR_FOUND)
    if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
        list (REMOVE_ITEM SOURCE_FILES ${TMP_PATH})
    endif ()
endforeach(TMP_PATH)
MESSAGE("Files to coveralls: ${SOURCE_FILES}")
coveralls_setup("${SOURCE_FILES}" ON "${CMAKE_SOURCE_DIR}/tools/cmake")
endif()
>>>>>>> Stashed changes
