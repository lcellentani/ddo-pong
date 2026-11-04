add_executable(tick_smoke tick_smoke_test.cpp)
target_link_libraries(tick_smoke PRIVATE ddo_core warnings)
add_test(NAME tick_smoke COMMAND tick_smoke)
