cmake_minimum_required(VERSION 4.0.0)

add_library(cyberverse_ui_static_lib STATIC)
add_library(cyberverse::ui::static_lib ALIAS cyberverse_ui_static_lib)

target_include_directories(cyberverse_ui_static_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")

set(
  HEADER_FILES
  cyberverse/ui.hpp
  cyberverse/ui/system.hpp
  cyberverse/ui/layer.hpp
  cyberverse/ui/panel.hpp
  cyberverse/ui/panel_context.hpp
  cyberverse/ui/frame_stage.hpp
  cyberverse/ui/font.hpp
  cyberverse/ui/theme.hpp
  cyberverse/ui/imgui.hpp
  cyberverse/ui/widgets.hpp
  cyberverse/ui/widgets/widget.hpp
  cyberverse/ui/widgets/theme.hpp
  cyberverse/ui/widgets/flexbox.hpp
  cyberverse/ui/widgets/panel.hpp
  cyberverse/ui/widgets/scroll_view.hpp
  cyberverse/ui/widgets/collapse_group.hpp
  cyberverse/ui/widgets/text.hpp
  cyberverse/ui/widgets/button.hpp
  cyberverse/ui/widgets/checkbox.hpp
  cyberverse/ui/widgets/radio.hpp
  cyberverse/ui/widgets/text_input.hpp
  cyberverse/ui/widgets/native.hpp
  cyberverse/ui/widgets/divider.hpp
  cyberverse/ui/widgets/stack.hpp
)

set(
  DETAIL_HEADER_FILES
  cyberverse/ui/detail/frame_phase.hpp
  cyberverse/ui/detail/layer_access.hpp
  cyberverse/ui/detail/input_bridge.hpp
  cyberverse/ui/detail/key_map.hpp
  cyberverse/ui/detail/system_impl.hpp
  cyberverse/ui/detail/z_order.hpp
  cyberverse/ui/widgets/detail/layout_types.hpp
  cyberverse/ui/widgets/detail/style_resolver.hpp
  cyberverse/ui/widgets/detail/draw_utils.hpp
  cyberverse/ui/widgets/detail/layout_engine.hpp
)

set(
  SOURCE_FILES
  cyberverse/ui/layer.cpp
  cyberverse/ui/frame_stage.cpp
  cyberverse/ui/system.cpp
  cyberverse/ui/detail/input_bridge.cpp
  cyberverse/ui/detail/key_map.cpp
  cyberverse/ui/detail/z_order.cpp
  cyberverse/ui/widgets/widget.cpp
  cyberverse/ui/widgets/flexbox.cpp
  cyberverse/ui/widgets/panel.cpp
  cyberverse/ui/widgets/scroll_view.cpp
  cyberverse/ui/widgets/collapse_group.cpp
  cyberverse/ui/widgets/text.cpp
  cyberverse/ui/widgets/button.cpp
  cyberverse/ui/widgets/checkbox.cpp
  cyberverse/ui/widgets/radio.cpp
  cyberverse/ui/widgets/text_input.cpp
  cyberverse/ui/widgets/native.cpp
  cyberverse/ui/widgets/divider.cpp
  cyberverse/ui/widgets/detail/style_resolver.cpp
  cyberverse/ui/widgets/detail/layout_engine.cpp
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

# Yoga is an implementation detail of the widget layout layer.
target_link_libraries(cyberverse_ui_static_lib PRIVATE yogacore)

# Functional widgets2 library. It is intentionally separate from the legacy
# Yoga-backed widgets target and does not depend on the old panel runtime.
add_library(cyberverse_ui_widgets2_static_lib STATIC)
add_library(cyberverse::ui::widgets2_static_lib ALIAS cyberverse_ui_widgets2_static_lib)
target_include_directories(cyberverse_ui_widgets2_static_lib PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}")
target_sources(cyberverse_ui_widgets2_static_lib PUBLIC
  cyberverse/ui/widgets2.hpp
  cyberverse/ui/widgets2/animator.hpp
  cyberverse/ui/widgets2/button.hpp
  cyberverse/ui/widgets2/component.hpp
  cyberverse/ui/widgets2/context.hpp
  cyberverse/ui/widgets2/detail/font.hpp
  cyberverse/ui/widgets2/detail/state_access.hpp
  cyberverse/ui/widgets2/event.hpp
  cyberverse/ui/widgets2/impl/button.hpp
  cyberverse/ui/widgets2/impl/text.hpp
  cyberverse/ui/widgets2/property.hpp
  cyberverse/ui/widgets2/rect.hpp
  cyberverse/ui/widgets2/run.hpp
  cyberverse/ui/widgets2/stack.hpp
  cyberverse/ui/widgets2/state.hpp
  cyberverse/ui/widgets2/text.hpp
  cyberverse/ui/widgets2/theme.hpp
  cyberverse/ui/widgets2/visibility.hpp
)
target_sources(cyberverse_ui_widgets2_static_lib PRIVATE
  cyberverse/ui/widgets2/state.cpp
  cyberverse/ui/widgets2/text.cpp
  cyberverse/ui/widgets2/button.cpp
)
target_compile_features(cyberverse_ui_widgets2_static_lib PUBLIC cxx_std_23)
target_link_libraries(
  cyberverse_ui_widgets2_static_lib
  PUBLIC
  Diligent-Imgui
  cyberverse::animation::static_lib
)
