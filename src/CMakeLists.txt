cmake_minimum_required(VERSION 4.0.0)

add_library(cyberverse_ui_static_lib STATIC)
add_library(cyberverse::ui::static_lib ALIAS cyberverse_ui_static_lib)

target_include_directories(cyberverse_ui_static_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set(
  HEADER_FILES
  cyberverse/ui.hpp
  cyberverse/ui/host.hpp
    cyberverse/ui/window.hpp
  cyberverse/ui/window_context.hpp
  cyberverse/ui/frame_stage.hpp
  cyberverse/ui/font.hpp
  cyberverse/ui/theme.hpp
  cyberverse/ui/imgui.hpp
  cyberverse/ui/rect.hpp
  cyberverse/ui/widgets.hpp
  cyberverse/ui/widgets/animator.hpp
  cyberverse/ui/widgets/box.hpp
  cyberverse/ui/widgets/box_types.hpp
  cyberverse/ui/widgets/button.hpp
  cyberverse/ui/widgets/checkbox.hpp
  cyberverse/ui/widgets/component.hpp
  cyberverse/ui/widgets/context.hpp
  cyberverse/ui/widgets/divider.hpp
  cyberverse/ui/widgets/divider_types.hpp
  cyberverse/ui/widgets/event.hpp
  cyberverse/ui/widgets/flex_types.hpp
  cyberverse/ui/widgets/flexbox.hpp
  cyberverse/ui/widgets/property.hpp
  cyberverse/ui/widgets/radio.hpp
  cyberverse/ui/widgets/run.hpp
  cyberverse/ui/widgets/scroll_view.hpp
  cyberverse/ui/widgets/scroll_view_types.hpp
  cyberverse/ui/widgets/stack.hpp
  cyberverse/ui/widgets/stack_types.hpp
  cyberverse/ui/widgets/state.hpp
  cyberverse/ui/widgets/text.hpp
  cyberverse/ui/widgets/text_input.hpp
  cyberverse/ui/widgets/theme.hpp
  cyberverse/ui/widgets/types.hpp
  cyberverse/ui/widgets/visibility.hpp
)

set(
  DETAIL_HEADER_FILES
  cyberverse/ui/detail/frame_phase.hpp
    cyberverse/ui/detail/input_bridge.hpp
  cyberverse/ui/detail/key_map.hpp
  cyberverse/ui/detail/host_impl.hpp
  cyberverse/ui/detail/z_order.hpp
  cyberverse/ui/widgets/detail/box_canvas.hpp
  cyberverse/ui/widgets/detail/font.hpp
  cyberverse/ui/widgets/detail/scroll_canvas.hpp
  cyberverse/ui/widgets/detail/state_access.hpp
  cyberverse/ui/widgets/impl/box.hpp
  cyberverse/ui/widgets/impl/button.hpp
  cyberverse/ui/widgets/impl/checkbox.hpp
  cyberverse/ui/widgets/impl/divider.hpp
  cyberverse/ui/widgets/impl/flexbox.hpp
  cyberverse/ui/widgets/impl/radio.hpp
  cyberverse/ui/widgets/impl/scroll_view.hpp
  cyberverse/ui/widgets/impl/stack.hpp
  cyberverse/ui/widgets/impl/text.hpp
  cyberverse/ui/widgets/impl/text_input.hpp
)

set(
  SOURCE_FILES
    cyberverse/ui/frame_stage.cpp
  cyberverse/ui/host.cpp
  cyberverse/ui/window.cpp
  cyberverse/ui/detail/input_bridge.cpp
  cyberverse/ui/detail/key_map.cpp
  cyberverse/ui/detail/z_order.cpp
  cyberverse/ui/widgets/state.cpp
  cyberverse/ui/widgets/detail/box_canvas.cpp
  cyberverse/ui/widgets/detail/scroll_canvas.cpp
  cyberverse/ui/widgets/impl/button.cpp
  cyberverse/ui/widgets/impl/checkbox.cpp
  cyberverse/ui/widgets/impl/divider.cpp
  cyberverse/ui/widgets/impl/radio.cpp
  cyberverse/ui/widgets/impl/text.cpp
  cyberverse/ui/widgets/impl/text_input.cpp
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
  cyberverse::animation::static_lib
  Diligent-Imgui
)
