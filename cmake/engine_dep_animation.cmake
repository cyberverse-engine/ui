include_guard(GLOBAL)
include(FetchContent)

# Resolve cyberverse::animation::static_lib for widgets2 animation support.
# Resolution order mirrors the other engine dependency modules.
function(resolve_engine_dep_animation)
  if(TARGET cyberverse::animation::static_lib)
    return()
  endif()

  set(CYBERVERSE_ANIMATION_BUILD_TESTS OFF CACHE BOOL "Build cyberverse_animation tests" FORCE)
  set(CYBERVERSE_ANIMATION_BUILD_BENCHMARKS OFF CACHE BOOL "Build cyberverse_animation benchmarks" FORCE)

  find_package(cyberverse_animation CONFIG QUIET)
  if(TARGET cyberverse::animation::static_lib)
    message(STATUS "cyberverse_animation: resolved from installed package")
    return()
  endif()

  set(_dir "${CYBERVERSE_ANIMATION_DIR}")
  if(NOT _dir AND DEFINED ENV{CYBERVERSE_ANIMATION_DIR})
    set(_dir "$ENV{CYBERVERSE_ANIMATION_DIR}")
  endif()
  if(NOT _dir)
    set(_dir "${CMAKE_CURRENT_FUNCTION_LIST_DIR}/../../animation")
  endif()
  get_filename_component(_dir "${_dir}" ABSOLUTE)

  if(_dir AND EXISTS "${_dir}/CMakeLists.txt")
    message(STATUS "cyberverse_animation: resolved from local source ${_dir}")
    add_subdirectory("${_dir}" "${CMAKE_BINARY_DIR}/_deps/animation_build")
    if(TARGET cyberverse::animation::static_lib)
      return()
    endif()
  endif()

  set(
    CYBERVERSE_ANIMATION_GIT_REPOSITORY
    "git@github.com:cyberverse-engine/animation.git"
    CACHE STRING "Git repository used to fetch cyberverse_animation"
  )
  set(
    CYBERVERSE_ANIMATION_GIT_TAG
    "main"
    CACHE STRING "Git tag, branch, or commit used to fetch cyberverse_animation"
  )
  option(CYBERVERSE_ANIMATION_GIT_SHALLOW "Use a shallow clone when fetching cyberverse_animation" ON)

  if(CYBERVERSE_ANIMATION_GIT_REPOSITORY)
    message(
      STATUS
      "cyberverse_animation: fetching ${CYBERVERSE_ANIMATION_GIT_REPOSITORY} (${CYBERVERSE_ANIMATION_GIT_TAG})"
    )
    FetchContent_Declare(
      cyberverse_animation
      GIT_REPOSITORY "${CYBERVERSE_ANIMATION_GIT_REPOSITORY}"
      GIT_TAG "${CYBERVERSE_ANIMATION_GIT_TAG}"
      GIT_SHALLOW "${CYBERVERSE_ANIMATION_GIT_SHALLOW}"
    )
    FetchContent_MakeAvailable(cyberverse_animation)
    if(TARGET cyberverse::animation::static_lib)
      message(STATUS "cyberverse_animation: resolved from fetched source")
      return()
    endif()
  endif()

  message(FATAL_ERROR
    "cyberverse_animation: cannot be resolved. Provide an installed package, set "
    "CYBERVERSE_ANIMATION_DIR to animation's source directory, or set "
    "CYBERVERSE_ANIMATION_GIT_REPOSITORY.")
endfunction()
