cmake_minimum_required(VERSION 4.0.0)

add_library(cyberverse_ui_static_lib STATIC)
add_library(cyberverse::ui::static_lib ALIAS cyberverse_ui_static_lib)

target_include_directories(cyberverse_ui_static_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set(
  HEADER_FILES
  cyberverse/ui.hpp
  cyberverse/ui/system.hpp
  cyberverse/ui/frame_stage.hpp
  cyberverse/ui/panel.hpp
  cyberverse/ui/panel_context.hpp
  cyberverse/ui/runtime_stage.hpp
  cyberverse/ui/font.hpp
  cyberverse/ui/theme.hpp
  cyberverse/ui/imgui.hpp
)

set(
  DETAIL_HEADER_FILES
  cyberverse/ui/detail/frame_phase.hpp
  cyberverse/ui/detail/frame_stage_access.hpp
  cyberverse/ui/detail/input_bridge.hpp
  cyberverse/ui/detail/key_map.hpp
  cyberverse/ui/detail/system_impl.hpp
  cyberverse/ui/detail/z_order.hpp
)

set(
  SOURCE_FILES
  cyberverse/ui/frame_stage.cpp
  cyberverse/ui/runtime_stage.cpp
  cyberverse/ui/system.cpp
  cyberverse/ui/detail/input_bridge.cpp
  cyberverse/ui/detail/key_map.cpp
  cyberverse/ui/detail/z_order.cpp
)

target_sources(cyberverse_ui_static_lib PUBLIC ${HEADER_FILES})
target_sources(cyberverse_ui_static_lib PRIVATE ${DETAIL_HEADER_FILES} ${SOURCE_FILES})

target_compile_features(cyberverse_ui_static_lib PUBLIC cxx_std_23)

# Diligent-Imgui is PUBLIC because cyberverse/ui/imgui.hpp is the application's
# explicit Dear ImGui entry point and the ui library links the only ImGui copy in
# the process. Applications must not link another ImGui.
target_link_libraries(
  cyberverse_ui_static_lib
  PUBLIC
  cyberverse::runtime::static_lib
  cyberverse::platform::static_lib
  cyberverse::render_core::static_lib
  Diligent-Imgui
)
