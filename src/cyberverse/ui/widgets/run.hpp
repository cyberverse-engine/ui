/**
 * @file run.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Fixed per-frame widgets entry point
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/context.hpp"
#include "cyberverse/ui/widgets/detail/state_access.hpp"
#include "cyberverse/ui/widgets/state.hpp"
#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Tick, measure, draw, and reserve one root component
 *
 * Must be called once per root per frame, between ImGui::NewFrame() and
 * ImGui::Render(), inside the current managed window. Animation time comes from
 * window.delta_time; a zero delta freezes all animations.
 *
 * @param window Current managed window context
 * @param appearance Resolved widget theme
 * @param root Root component
 * @param opacity Inherited opacity for the whole root subtree
 *
 */
template <component_like T>
layout_size run(window_context& window, const theme& appearance, T& root, float opacity = 1.0f) {
  float initial_opacity = 1.0f;
  if (std::isfinite(opacity)) {
    initial_opacity = std::clamp(opacity, 0.0f, 1.0f);
  }
  widget_context context{ window, appearance, initial_opacity };
  root.tick(context);

  const ImVec2 origin = ImGui::GetCursorScreenPos();
  const ImVec2 available = ImGui::GetContentRegionAvail();
  const constraints limits{ available.x, available.y };

  const layout_size measured = root.measure(appearance, limits);
  const float width = std::max(0.0f, measured.width);
  const float height = std::max(0.0f, measured.height);
  detail::state_access::set_bounds(root.s, rect{ origin.x, origin.y, width, height });

  root.draw(context);

  ImGui::SetCursorScreenPos(origin);
  ImGui::Dummy(ImVec2{ std::max(1.0f, width), std::max(1.0f, height) });
  return measured;
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
