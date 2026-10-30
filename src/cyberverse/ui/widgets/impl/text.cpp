/**
 * @file text.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets text implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/impl/text.hpp"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/font.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

layout_size text::measure(const state& s, const theme& th, const constraints& /*lim*/) const noexcept {
  detail::font_scope font(detail::resolve_font_size(s, th));
  const ImVec2 measured = ImGui::CalcTextSize(value.data(), value.data() + value.size());

  float width = std::max(0.0f, measured.x);
  float height = std::max(0.0f, measured.y);
  if (s.has(property_type::width)) {
    width = std::max(0.0f, s.value(property_type::width));
  }
  if (s.has(property_type::height)) {
    height = std::max(0.0f, s.value(property_type::height));
  }
  return layout_size{ width, height };
}

void text::draw(state& s, draw_context& ctx) const {
  const rect& bounds = s.bounds();
  if (value.empty() || bounds.width <= 0.0f || bounds.height <= 0.0f) {
    return;
  }

  const text_style& style = style_override ? *style_override : ctx.appearance.text;
  const detail::font_scope font(detail::resolve_font_size(s, ctx.appearance));

  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  draw_list->PushClipRect(
      ImVec2{ bounds.x, bounds.y }, ImVec2{ bounds.x + bounds.width, bounds.y + bounds.height }, true);
  draw_list->AddText(ImGui::GetFont(),
                     ImGui::GetFontSize(),
                     ImVec2{ bounds.x, bounds.y },
                     detail::to_u32(style.text, ctx.opacity),
                     value.data(),
                     value.data() + value.size());
  draw_list->PopClipRect();
}

} // namespace impl
} // namespace widgets
} // namespace ui
} // namespace cyberverse
