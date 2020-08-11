add_static_library()
target_link_libraries(${SIM_CURRENT_TARGET} PUBLIC SimulatorBase CONAN_PKG::boost Classes Rotations CONAN_PKG::tbb)
target_link_libraries(${SIM_CURRENT_TEST} PUBLIC ${SIM_CURRENT_TARGET})
target_compile_definitions(${SIM_CURRENT_TEST} PUBLIC SIM_TEST_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\")