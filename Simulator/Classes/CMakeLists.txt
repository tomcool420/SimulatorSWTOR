add_static_library()
target_link_libraries(${SIM_CURRENT_TARGET} PUBLIC SimulatorBase)
target_link_libraries(${SIM_CURRENT_TEST} PUBLIC ${SIM_CURRENT_TARGET})
