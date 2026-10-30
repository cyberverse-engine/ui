/**
 * @file divider.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets divider implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/impl/divider.hpp"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/font.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

layout_size divider::measure(const state& s, const theme& /*th*/, const constraints& lim) const noexcept {
  const float thickness = std::isfinite(options.thickness) ? std::max(0.0f, options.thickness) : 0.0f;
  const bool horizontal = options.orientation == divider_orientation::horizontal;

  float width = horizontal ? std::max(0.0f, lim.available_width) : thickness;
  float height = horizontal ? thickness : std::max(0.0f, lim.available_height);

  if (s.has(property_type::width)) {
    width = std::max(0.0f, s.value(property_type::width));
  }
  if (s.has(property_type::height)) {
    height = std::max(0.0f, s.value(property_type::height));
  }
  return layout_size{ width, height };
}

void divider::draw(state& s, draw_context& ctx) const {
  const rect& bounds = s.bounds();
  const float thickness = std::isfinite(options.thickness) ? std::max(0.0f, options.thickness) : 0.0f;
  if (thickness <= 0.0f || bounds.width <= 0.0f || bounds.height <= 0.0f) {
    return;
  }

  const divider_style& style = style_override ? *style_override : ctx.appearance.divider;
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  const ImU32 color = detail::to_u32(style.color, ctx.opacity);

  if (options.orientation == divider_orientation::horizontal) {
    const float y = bounds.y + bounds.height * 0.5f;
    draw_list->AddLine(ImVec2{ bounds.x, y }, ImVec2{ bounds.x + bounds.width, y }, color, thickness);
  } else {
    const float x = bounds.x + bounds.width * 0.5f;
    draw_list->AddLine(ImVec2{ x, bounds.y }, ImVec2{ x, bounds.y + bounds.height }, color, thickness);
  }
}

} // namespace impl
} // namespace widgets
} // namespace ui
} // namespace cyberverse
