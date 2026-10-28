/**
 * @file text.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Text display widget implementation
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/text.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/draw_utils.hpp"
#include "cyberverse/ui/widgets/detail/layout_types.hpp"
#include "cyberverse/ui/widgets/detail/style_resolver.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

text::text(std::string value) : value_{ std::move(value) } {}

text& text::wrap(bool enabled) {
  wrap_ = enabled;
  return *this;
}

text& text::color(widgets::color value) {
  color_ = value;
  return *this;
}

detail::measured_size text::measure(detail::measure_context& context, const detail::constraints& limits) {
  const theme& appearance = detail::appearance_or_default(context.appearance);
  const detail::resolved_style style = detail::resolve_style(detail::style_kind::text, appearance, style_override_);

  float wrap_width = 0.0f;
  if (wrap_ && limits.width_mode != detail::measure_mode::undefined && limits.available_width > 0.0f) {
    wrap_width = std::max(0.0f, limits.available_width - 2.0f * style.padding_x);
  }

  const ImVec2 measured =
      wrap_ ? ImGui::CalcTextSize(value_.c_str(), nullptr, false, wrap_width) : ImGui::CalcTextSize(value_.c_str());
  return detail::measured_size{ measured.x + 2.0f * style.padding_x, measured.y + 2.0f * style.padding_y };
}

void text::draw(detail::draw_context& context) {
  if (value_.empty() || bounds_.width <= 0.0f || bounds_.height <= 0.0f || context.draw_list == nullptr) {
    return;
  }

  const detail::resolved_style style =
      detail::resolve_style(detail::style_kind::text, context.appearance, style_override_);
  const ImU32 text_color = color_ ? detail::to_u32(*color_) : style.text;

  const float wrap_width = wrap_ ? std::max(0.0f, bounds_.width - 2.0f * style.padding_x) : 0.0f;
  detail::clip_rect(context.draw_list, bounds_);
  context.draw_list->AddText(ImGui::GetFont(),
                             ImGui::GetFontSize(),
                             ImVec2{ bounds_.x + style.padding_x, bounds_.y + style.padding_y },
                             text_color,
                             value_.c_str(),
                             nullptr,
                             wrap_width);
  detail::unclip_rect(context.draw_list);
}

void text::visit_children(const child_visitor& /*visitor*/) const {}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
