cmake_minimum_required(VERSION 4.0.0)
project(cyberverse_ui CXX)

option(CYBERVERSE_UI_BUILD_TESTS "Build unit test cases" ON)
option(CYBERVERSE_UI_BUILD_BENCHMARKS "Build benchmarks" OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# cyberverse::ui hard-depends on Dear ImGui and DiligentEngine. ImGui comes from
# DiligentEngine's DiligentTools (Diligent-Imgui); there is no backend fallback.
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(engine_dep_platform)
include(engine_dep_runtime)
include(engine_dep_render_core)
include(engine_dep_yoga)

resolve_engine_dep_platform()
resolve_engine_dep_runtime()
resolve_engine_dep_render_core()
resolve_engine_dep_yoga()

if(NOT TARGET Diligent-Imgui)
  message(FATAL_ERROR
    "cyberverse_ui: the Diligent-Imgui target is unavailable. cyberverse::ui "
    "hard-depends on Dear ImGui through DiligentEngine's DiligentTools. "
    "Provide CYBERVERSE_DILIGENT_DIR pointing at a DiligentEngine source tree and keep "
    "DILIGENT_BUILD_TOOLS=ON (render-core enables it by default), or build from a "
    "superproject that already defines the Diligent-Imgui target.")
endif()

add_subdirectory(src)

if(CYBERVERSE_UI_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

if(CYBERVERSE_UI_BUILD_BENCHMARKS)
  add_subdirectory(benchmark)
endif()
