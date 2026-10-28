include_guard(GLOBAL)
include(FetchContent)

# Resolve the Yoga flexbox layout library used by cyberverse::ui::widgets.
#
# Yoga is an implementation-private dependency: it is linked into the ui static
# library but never appears in a public header. Resolution order:
# 1. An already-defined yogacore target (superproject or an earlier call).
# 2. CYBERVERSE_UI_YOGA_DIR pointing at a Yoga source tree.
# 3. FetchContent from CYBERVERSE_UI_YOGA_GIT_REPOSITORY (default: v3.2.1).
#
# SOURCE_SUBDIR yoga builds only Yoga's library directory; it skips Yoga's
# top-level CMakeLists, tests, and its GoogleTest download.
function(resolve_engine_dep_yoga)
  if(TARGET yogacore)
    return()
  endif()

  set(CYBERVERSE_UI_YOGA_DIR "" CACHE PATH "Yoga source tree (optional)")

  if(CYBERVERSE_UI_YOGA_DIR)
    get_filename_component(_yoga_dir "${CYBERVERSE_UI_YOGA_DIR}" ABSOLUTE)
    if(NOT EXISTS "${_yoga_dir}/yoga/CMakeLists.txt")
      message(FATAL_ERROR "cyberverse_ui: CYBERVERSE_UI_YOGA_DIR does not contain yoga/CMakeLists.txt")
    endif()
    message(STATUS "cyberverse_yoga: resolved from local source ${_yoga_dir}")
    add_subdirectory("${_yoga_dir}/yoga" "${CMAKE_BINARY_DIR}/_deps/yoga_build" EXCLUDE_FROM_ALL)
  else()
    set(
      CYBERVERSE_UI_YOGA_GIT_REPOSITORY
      "https://github.com/facebook/yoga.git"
      CACHE STRING "Git repository used to fetch yoga"
    )
    set(CYBERVERSE_UI_YOGA_GIT_TAG "v3.2.1" CACHE STRING "Git tag, branch, or commit used to fetch yoga")
    option(CYBERVERSE_UI_YOGA_GIT_SHALLOW "Use a shallow clone when fetching yoga" ON)

    message(
      STATUS
      "cyberverse_yoga: fetching ${CYBERVERSE_UI_YOGA_GIT_REPOSITORY} (${CYBERVERSE_UI_YOGA_GIT_TAG})"
    )
    FetchContent_Declare(
      cyberverse_yoga
      GIT_REPOSITORY "${CYBERVERSE_UI_YOGA_GIT_REPOSITORY}"
      GIT_TAG "${CYBERVERSE_UI_YOGA_GIT_TAG}"
      GIT_SHALLOW "${CYBERVERSE_UI_YOGA_GIT_SHALLOW}"
      SOURCE_SUBDIR yoga
    )
    FetchContent_MakeAvailable(cyberverse_yoga)
  endif()

  if(NOT TARGET yogacore)
    message(FATAL_ERROR
      "cyberverse_ui: the Yoga 'yogacore' target is unavailable. "
      "Set CYBERVERSE_UI_YOGA_DIR to a Yoga source tree or provide a superproject "
      "that defines yogacore.")
  endif()
endfunction()
