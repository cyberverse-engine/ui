/**
 * @file native.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native ImGui content escape hatch implementation
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/native.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/draw_utils.hpp"
#include "cyberverse/ui/widgets/detail/layout_types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

native::native(std::string id, draw_fn draw, measure_fn measure)
    : draw_{ std::move(draw) }, measure_{ std::move(measure) } {
  id_ = std::move(id);
}

detail::measured_size native::measure(detail::measure_context& /*context*/, const detail::constraints& limits) {
  if (measure_) {
    const content_size measured = measure_();
    return detail::measured_size{ std::max(0.0f, measured.width), std::max(0.0f, measured.height) };
  }

  return detail::measured_size{ std::max(0.0f, limits.available_width), std::max(0.0f, limits.available_height) };
}

void native::draw(detail::draw_context& context) {
  if (!draw_ || context.draw_list == nullptr || bounds_.width <= 0.0f || bounds_.height <= 0.0f) {
    return;
  }

  detail::clip_rect(context.draw_list, bounds_);
  ImGui::SetCursorScreenPos(detail::rect_min(bounds_));
  draw_(bounds_);
  detail::unclip_rect(context.draw_list);
}

void native::visit_children(const child_visitor& /*visitor*/) const {}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
