/**
 * @file button.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets button implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/impl/button.hpp"

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/font.hpp"
#include "cyberverse/ui/widgets/event.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

layout_size button::measure(const state& s, const theme& th, const constraints& /*lim*/) const noexcept {
  const button_style& style = style_override ? *style_override : th.button;
  const detail::font_scope font(detail::resolve_font_size(s, th));
  const ImVec2 text_size = ImGui::CalcTextSize(label.data(), label.data() + label.size());

  float width = std::max(0.0f, text_size.x + 2.0f * style.padding_x);
  float height = std::max(0.0f, text_size.y + 2.0f * style.padding_y);
  height = std::max(height, ImGui::GetFrameHeight());

  if (s.has(property_type::width)) {
    width = std::max(0.0f, s.value(property_type::width));
  }
  if (s.has(property_type::height)) {
    height = std::max(0.0f, s.value(property_type::height));
  }
  return layout_size{ width, height };
}

void button::draw(state& s, widget_context& ctx) const {
  const rect& bounds = s.bounds();
  if (bounds.width <= 0.0f || bounds.height <= 0.0f) {
    return;
  }

  const button_style& style = style_override ? *style_override : ctx.appearance.button;
  const detail::font_scope font(detail::resolve_font_size(s, ctx.appearance));

  ImGui::PushID(s.id().data(), s.id().data() + s.id().size());
  ImGui::PushStyleColor(ImGuiCol_Button, detail::to_u32(style.background));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, detail::to_u32(style.background_hovered));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, detail::to_u32(style.background_active));
  ImGui::PushStyleColor(ImGuiCol_Border, detail::to_u32(style.background));
  ImGui::PushStyleColor(ImGuiCol_Text, detail::to_u32(style.text));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.rounding);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.border_width);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ style.padding_x, style.padding_y });
  ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(ctx.opacity, 0.0f, 1.0f));

  ImGui::SetCursorScreenPos(ImVec2{ bounds.x, bounds.y });
  const bool clicked =
      ImGui::Button(label.data(), ImVec2{ std::max(1.0f, bounds.width), std::max(1.0f, bounds.height) });

  ImGui::PopStyleVar(4);
  ImGui::PopStyleColor(5);
  ImGui::PopID();

  if (clicked) {
    s.emit(event_type::click);
  }
}

} // namespace impl
} // namespace widgets
} // namespace ui
} // namespace cyberverse
