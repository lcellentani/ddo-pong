add_executable(tick_smoke tick_smoke_test.cpp)
target_link_libraries(tick_smoke PRIVATE ddo_core warnings)
add_test(NAME tick_smoke COMMAND tick_smoke)

if(DDO_SANITIZER_CANARIES)
  add_executable(sanitizer_canary sanitizer_canary.cpp)
  add_test(NAME asan_canary COMMAND sanitizer_canary heap)
  add_test(NAME ubsan_canary COMMAND sanitizer_canary int)
  set_tests_properties(asan_canary PROPERTIES
    PASS_REGULAR_EXPRESSION "heap-buffer-overflow"
    FAIL_REGULAR_EXPRESSION "canary survived")
  set_tests_properties(ubsan_canary PROPERTIES
    PASS_REGULAR_EXPRESSION "signed integer overflow"
    FAIL_REGULAR_EXPRESSION "canary survived")
endif()
