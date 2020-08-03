add_exe()
target_link_libraries(${SIM_CURRENT_TARGET} PUBLIC libSimulator CONAN_PKG::boost libSimulator CONAN_PKG::spdlog)
