include_guard(GLOBAL)
include(FetchContent)

# Resolve cyberverse::render_core::static_lib for the UI runtime.
# Idempotent: calling it more than once is safe.
#
# Resolution order:
# 1. An already-defined target (superproject or an earlier call).
# 2. An installed cyberverse_render_core CMake package.
# 3. A render-core source tree: CYBERVERSE_RENDER_CORE_DIR, its environment
#    equivalent, or the sibling ../../render-core directory.
# 4. FetchContent from CYBERVERSE_RENDER_CORE_GIT_REPOSITORY.
#
# render-core resolves its own DiligentEngine/geometry/platform/runtime
# dependencies. It enables DiligentTools by default, which is what provides the
# Diligent-Imgui target the UI runtime requires.
function(resolve_engine_dep_render_core)
  if(TARGET cyberverse::render_core::static_lib)
    return()
  endif()

  # render-core is a dependency here, so never build its tests or benchmarks.
  set(CYBERVERSE_RENDER_CORE_BUILD_TESTS OFF CACHE BOOL "Build cyberverse_render_core tests" FORCE)
  set(CYBERVERSE_RENDER_CORE_BUILD_BENCHMARKS OFF CACHE BOOL "Build cyberverse_render_core benchmarks" FORCE)

  find_package(cyberverse_render_core CONFIG QUIET)

  if(TARGET cyberverse::render_core::static_lib)
    message(STATUS "cyberverse_render_core: resolved from installed package")
    return()
  endif()

  set(_dir "${CYBERVERSE_RENDER_CORE_DIR}")

  if(NOT _dir AND DEFINED ENV{CYBERVERSE_RENDER_CORE_DIR})
    set(_dir "$ENV{CYBERVERSE_RENDER_CORE_DIR}")
  endif()

  if(NOT _dir)
    set(_dir "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../render-core")
  endif()

  get_filename_component(_dir "${_dir}" ABSOLUTE)

  if(_dir AND EXISTS "${_dir}/CMakeLists.txt")
    message(STATUS "cyberverse_render_core: resolved from local source ${_dir}")
    add_subdirectory("${_dir}" "${CMAKE_BINARY_DIR}/_deps/render_core_build")

    if(TARGET cyberverse::render_core::static_lib)
      return()
    endif()
  elseif(CYBERVERSE_RENDER_CORE_DIR)
    message(
      STATUS
      "cyberverse_render_core: CYBERVERSE_RENDER_CORE_DIR does not contain a CMakeLists.txt, falling back"
    )
  endif()

  # Use the public GitHub host; local SSH aliases are not portable.
  set(
    CYBERVERSE_RENDER_CORE_GIT_REPOSITORY
    "git@github.com:cyberverse-engine/render-core.git"
    CACHE STRING "Git repository used to fetch cyberverse_render_core"
  )
  set(
    CYBERVERSE_RENDER_CORE_GIT_TAG
    "main"
    CACHE STRING "Git tag, branch, or commit used to fetch cyberverse_render_core"
  )
  option(CYBERVERSE_RENDER_CORE_GIT_SHALLOW "Use a shallow clone when fetching cyberverse_render_core" ON)

  if(CYBERVERSE_RENDER_CORE_GIT_REPOSITORY)
    message(
      STATUS
      "cyberverse_render_core: fetching ${CYBERVERSE_RENDER_CORE_GIT_REPOSITORY} (${CYBERVERSE_RENDER_CORE_GIT_TAG})"
    )
    FetchContent_Declare(
      cyberverse_render_core
      GIT_REPOSITORY "${CYBERVERSE_RENDER_CORE_GIT_REPOSITORY}"
      GIT_TAG "${CYBERVERSE_RENDER_CORE_GIT_TAG}"
      GIT_SHALLOW "${CYBERVERSE_RENDER_CORE_GIT_SHALLOW}"
    )
    FetchContent_MakeAvailable(cyberverse_render_core)

    if(TARGET cyberverse::render_core::static_lib)
      message(STATUS "cyberverse_render_core: resolved from fetched source")
      return()
    endif()
  endif()

  message(FATAL_ERROR
    "cyberverse_render_core: cannot be resolved. "
    "Provide an installed cyberverse_render_core package, set CYBERVERSE_RENDER_CORE_DIR "
    "to render-core's source directory, or set CYBERVERSE_RENDER_CORE_GIT_REPOSITORY.")
endfunction()
