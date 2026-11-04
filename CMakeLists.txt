cmake_minimum_required(VERSION 3.25)
project(ddo_pong LANGUAGES C CXX)

option(DDO_TIDY "Run clang-tidy during build" OFF)
option(DDO_SANITIZER_CANARIES "Add tests that prove the sanitizers are active" OFF)

include(FetchContent)
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib.git
  GIT_TAG 6.2
  GIT_SHALLOW TRUE
  SYSTEM)
FetchContent_MakeAvailable(raylib)

add_library(warnings INTERFACE)
target_compile_options(warnings INTERFACE
  -Wall -Wextra -Wpedantic -Wshadow -Wconversion -Wsign-conversion
  -Wold-style-cast -Wnon-virtual-dtor -Wdouble-promotion)

# Pure simulation: no platform includes allowed here.
add_library(ddo_core STATIC src/sim/tick.cpp)
target_include_directories(ddo_core PUBLIC src)
target_compile_features(ddo_core PUBLIC cxx_std_20)
target_link_libraries(ddo_core PRIVATE warnings)

# Platform seam: the only target that links the window/render library.
add_library(ddo_platform STATIC src/platform/platform.cpp)
target_link_libraries(ddo_platform PUBLIC ddo_core PRIVATE raylib warnings)

add_executable(ddo_pong src/main.cpp)
target_link_libraries(ddo_pong PRIVATE ddo_core ddo_platform warnings)

add_custom_target(run COMMAND ddo_pong USES_TERMINAL)

enable_testing()
add_subdirectory(tests)

if(DDO_TIDY)
  set_target_properties(ddo_core ddo_platform ddo_pong tick_smoke PROPERTIES
    CXX_CLANG_TIDY "clang-tidy;--warnings-as-errors=*")
endif()

