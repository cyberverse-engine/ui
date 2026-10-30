cmake_minimum_required(VERSION 4.0.0)

include(FetchContent)

FetchContent_Declare(
  Catch2
  GIT_REPOSITORY https://github.com/catchorg/Catch2.git
  GIT_TAG v3.5.2
  GIT_SHALLOW ON
)
FetchContent_MakeAvailable(Catch2)

add_executable(
  cyberverse_ui_tests
  architecture_test.cpp
  font_test.cpp
  gpu_smoke_test.cpp
  input_bridge_test.cpp
  layer_test.cpp
  panel_test.cpp
  frame_stage_test.cpp
  system_test.cpp
  widgets_test.cpp
  widgets_interaction_test.cpp
  widgets_containers_test.cpp
  widgets_text_input_test.cpp
  z_order_test.cpp
  widgets2_test.cpp
)

target_link_libraries(
  cyberverse_ui_tests
  PRIVATE
  cyberverse::ui::static_lib
  cyberverse::ui::widgets2_static_lib
  Catch2::Catch2WithMain
)

list(APPEND CMAKE_MODULE_PATH ${catch2_SOURCE_DIR}/extras)
include(Catch)
catch_discover_tests(cyberverse_ui_tests PROPERTIES SKIP_RETURN_CODE 4)
