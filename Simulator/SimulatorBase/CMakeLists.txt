add_static_library()
target_link_libraries(${SIM_CURRENT_TARGET} PUBLIC CONAN_PKG::boost CONAN_PKG::ms-gsl CONAN_PKG::spdlog CONAN_PKG::nlohmann_json)
target_link_libraries(${SIM_CURRENT_TEST} PUBLIC ${SIM_CURRENT_TARGET})
