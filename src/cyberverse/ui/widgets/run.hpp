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
 * ImGui::Render(), inside the current window.
 *
 */
template <component_like T>
size run(T& root, const frame_context& frame) {
  update_context update{ frame.dt, frame.animations };
  root.tick(update);

  const ImVec2 origin = ImGui::GetCursorScreenPos();
  const ImVec2 available = ImGui::GetContentRegionAvail();
  const constraints limits{ available.x, available.y };

  const size measured = root.measure(frame.appearance, limits);
  const float width = std::max(0.0f, measured.width);
  const float height = std::max(0.0f, measured.height);
  detail::state_access::set_bounds(root.s, rect{ origin.x, origin.y, width, height });

  draw_context draw{ frame.appearance, 1.0f };
  root.draw(draw);

  ImGui::SetCursorScreenPos(origin);
  ImGui::Dummy(ImVec2{ std::max(1.0f, width), std::max(1.0f, height) });
  return measured;
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
