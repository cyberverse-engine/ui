include_guard(GLOBAL)
include(FetchContent)

# Resolve cyberverse::runtime::static_lib for the UI framework.
# Idempotent: calling it more than once is safe.
#
# Resolution order:
# 1. An already-defined target (superproject or an earlier call).
# 2. An installed cyberverse_runtime CMake package.
# 3. A runtime source tree: CYBERVERSE_RUNTIME_DIR, its environment equivalent,
#    or the sibling ../../runtime directory.
# 4. FetchContent from CYBERVERSE_RUNTIME_GIT_REPOSITORY.
function(resolve_engine_dep_runtime)
  if(TARGET cyberverse::runtime::static_lib)
    return()
  endif()

  # Runtime is a dependency here, so never build its tests or benchmarks.
  set(CYBERVERSE_RUNTIME_BUILD_TESTS OFF CACHE BOOL "Build cyberverse_runtime tests" FORCE)
  set(CYBERVERSE_RUNTIME_BUILD_BENCHMARKS OFF CACHE BOOL "Build cyberverse_runtime benchmarks" FORCE)

  find_package(cyberverse_runtime CONFIG QUIET)

  if(TARGET cyberverse::runtime::static_lib)
    message(STATUS "cyberverse_runtime: resolved from installed package")
    return()
  endif()

  set(_dir "${CYBERVERSE_RUNTIME_DIR}")

  if(NOT _dir AND DEFINED ENV{CYBERVERSE_RUNTIME_DIR})
    set(_dir "$ENV{CYBERVERSE_RUNTIME_DIR}")
  endif()

  if(NOT _dir)
    set(_dir "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../runtime")
  endif()

  get_filename_component(_dir "${_dir}" ABSOLUTE)

  if(_dir AND EXISTS "${_dir}/CMakeLists.txt")
    message(STATUS "cyberverse_runtime: resolved from local source ${_dir}")
    add_subdirectory("${_dir}" "${CMAKE_BINARY_DIR}/_deps/runtime_build")

    if(TARGET cyberverse::runtime::static_lib)
      return()
    endif()
  elseif(CYBERVERSE_RUNTIME_DIR)
    message(STATUS "cyberverse_runtime: CYBERVERSE_RUNTIME_DIR does not contain a CMakeLists.txt, falling back")
  endif()

  # Use the public GitHub host; local SSH aliases are not portable.
  set(
    CYBERVERSE_RUNTIME_GIT_REPOSITORY
    "git@github.com:cyberverse-engine/runtime.git"
    CACHE STRING "Git repository used to fetch cyberverse_runtime"
  )
  set(
    CYBERVERSE_RUNTIME_GIT_TAG
    "main"
    CACHE STRING "Git tag, branch, or commit used to fetch cyberverse_runtime"
  )
  option(CYBERVERSE_RUNTIME_GIT_SHALLOW "Use a shallow clone when fetching cyberverse_runtime" ON)

  if(CYBERVERSE_RUNTIME_GIT_REPOSITORY)
    message(
      STATUS
      "cyberverse_runtime: fetching ${CYBERVERSE_RUNTIME_GIT_REPOSITORY} (${CYBERVERSE_RUNTIME_GIT_TAG})"
    )
    FetchContent_Declare(
      cyberverse_runtime
      GIT_REPOSITORY "${CYBERVERSE_RUNTIME_GIT_REPOSITORY}"
      GIT_TAG "${CYBERVERSE_RUNTIME_GIT_TAG}"
      GIT_SHALLOW "${CYBERVERSE_RUNTIME_GIT_SHALLOW}"
    )
    FetchContent_MakeAvailable(cyberverse_runtime)

    if(TARGET cyberverse::runtime::static_lib)
      message(STATUS "cyberverse_runtime: resolved from fetched source")
      return()
    endif()
  endif()

  message(FATAL_ERROR
    "cyberverse_runtime: cannot be resolved. "
    "Provide an installed cyberverse_runtime package, set CYBERVERSE_RUNTIME_DIR to "
    "runtime's source directory, or set CYBERVERSE_RUNTIME_GIT_REPOSITORY.")
endfunction()
