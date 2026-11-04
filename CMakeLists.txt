cmake_minimum_required(VERSION 3.25)
project(ddo_pong LANGUAGES C CXX)

option(DDO_TIDY "Run clang-tidy during build" OFF)

add_library(warnings INTERFACE)
target_compile_options(warnings INTERFACE
  -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
  -Wold-style-cast -Wnon-virtual-dtor -Wdouble-promotion)

# Pure simulation: no platform includes allowed here.
add_library(ddo_core STATIC src/sim/tick.cpp)
target_include_directories(ddo_core PUBLIC src)
target_compile_features(ddo_core PUBLIC cxx_std_20)
target_link_libraries(ddo_core PRIVATE warnings)

add_executable(ddo_pong src/main.cpp)
target_link_libraries(ddo_pong PRIVATE ddo_core warnings)

if(DDO_TIDY)
  set_target_properties(ddo_core ddo_pong PROPERTIES
    CXX_CLANG_TIDY "clang-tidy")
endif()

enable_testing()
add_subdirectory(tests)
