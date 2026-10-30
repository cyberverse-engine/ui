/**
 * @file divider.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Lightweight horizontal or vertical line widget implementation
 * @version 0.4
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/divider.hpp"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/draw_utils.hpp"
#include "cyberverse/ui/widgets/detail/layout_types.hpp"
#include "cyberverse/ui/widgets/detail/style_resolver.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

namespace {

[[nodiscard]] float sanitize_thickness(float value) noexcept {
  return (std::isfinite(value) && value > 0.0f) ? value : 0.0f;
}

} // namespace

divider::divider(divider_orientation orientation) : orientation_{ orientation } { apply_orientation_defaults(); }

divider& divider::orientation(divider_orientation value) {
  orientation_ = value;
  apply_orientation_defaults();
  return *this;
}

divider& divider::thickness(float value) {
  thickness_ = sanitize_thickness(value);
  if (orientation_ == divider_orientation::horizontal) {
    height_ = size_spec{ size_mode::fixed, thickness_ };
  } else {
    width_ = size_spec{ size_mode::fixed, thickness_ };
  }
  return *this;
}

divider& divider::color(widgets::color value) {
  color_ = value;
  return *this;
}

detail::measured_size divider::measure(detail::measure_context& /*context*/, const detail::constraints& /*limits*/) {
  if (orientation_ == divider_orientation::horizontal) {
    return detail::measured_size{ 0.0f, thickness_ };
  }
  return detail::measured_size{ thickness_, 0.0f };
}

void divider::draw(detail::draw_context& context) {
  if (context.draw_list == nullptr || thickness_ <= 0.0f || bounds_.width <= 0.0f || bounds_.height <= 0.0f) {
    return;
  }

  const detail::resolved_style style =
      detail::resolve_style(detail::style_kind::divider, context.appearance, style_override_);
  const ImU32 line_color = color_ ? detail::to_u32(*color_) : style.border;
  detail::clip_rect(context.draw_list, bounds_);

  if (orientation_ == divider_orientation::horizontal) {
    const float y = bounds_.y + bounds_.height * 0.5f;
    context.draw_list->AddLine(ImVec2{ bounds_.x, y }, ImVec2{ bounds_.x + bounds_.width, y }, line_color, thickness_);
  } else {
    const float x = bounds_.x + bounds_.width * 0.5f;
    context.draw_list->AddLine(ImVec2{ x, bounds_.y }, ImVec2{ x, bounds_.y + bounds_.height }, line_color, thickness_);
  }

  detail::unclip_rect(context.draw_list);
}

void divider::visit_children(const child_visitor& /*visitor*/) const {}

void divider::apply_orientation_defaults() {
  if (orientation_ == divider_orientation::horizontal) {
    width_ = size_spec{ size_mode::stretch, 0.0f };
    height_ = size_spec{ size_mode::fixed, thickness_ };
  } else {
    width_ = size_spec{ size_mode::fixed, thickness_ };
    height_ = size_spec{ size_mode::stretch, 0.0f };
  }
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
