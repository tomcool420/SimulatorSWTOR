add_static_library()
target_link_libraries(${SIM_CURRENT_TARGET} PUBLIC SimulatorBase CONAN_PKG::boost Classes Rotations CONAN_PKG::spdlog)
