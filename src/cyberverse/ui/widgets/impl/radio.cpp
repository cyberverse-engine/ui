/**
 * @file radio.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets radio implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/impl/radio.hpp"

#include <algorithm>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/font.hpp"
#include "cyberverse/ui/widgets/event.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

size radio::measure(const state& s, const theme& th, const constraints& /*lim*/) const noexcept {
  const choice_style& style = style_override ? *style_override : th.radio;
  const detail::font_scope font(detail::resolve_font_size(s, th));
  const float frame_height = ImGui::GetFrameHeight();
  const ImVec2 label_size = ImGui::CalcTextSize(label.c_str());
  const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;

  float width = frame_height + spacing + label_size.x + 2.0f * style.padding_x;
  float height = std::max(frame_height, label_size.y) + 2.0f * style.padding_y;
  if (s.has(property_type::width)) {
    width = std::max(0.0f, s.value(property_type::width));
  }
  if (s.has(property_type::height)) {
    height = std::max(0.0f, s.value(property_type::height));
  }
  return size{ std::max(0.0f, width), std::max(0.0f, height) };
}

void radio::draw(state& s, draw_context& ctx) const {
  const rect& bounds = s.bounds();
  if (selected == nullptr || bounds.width <= 0.0f || bounds.height <= 0.0f) {
    return;
  }

  const choice_style& style = style_override ? *style_override : ctx.appearance.radio;
  const detail::font_scope font(detail::resolve_font_size(s, ctx.appearance));

  ImGui::PushID(s.id().data(), s.id().data() + s.id().size());
  ImGui::PushStyleColor(ImGuiCol_FrameBg, detail::to_u32(style.frame));
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, detail::to_u32(style.frame_hovered));
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, detail::to_u32(style.frame_active));
  ImGui::PushStyleColor(ImGuiCol_Border, detail::to_u32(style.border));
  ImGui::PushStyleColor(ImGuiCol_Text, detail::to_u32(style.text));
  ImGui::PushStyleColor(ImGuiCol_CheckMark, detail::to_u32(style.check));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.rounding);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.border_width);
  ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(ctx.opacity, 0.0f, 1.0f));

  ImGui::SetCursorScreenPos(ImVec2{ bounds.x + style.padding_x, bounds.y + style.padding_y });
  const bool clicked = ImGui::RadioButton(label.c_str(), *selected == value);

  ImGui::PopStyleVar(3);
  ImGui::PopStyleColor(6);
  ImGui::PopID();

  if (clicked) {
    *selected = value;
    s.emit(event_type::value_changed);
  }
}

} // namespace impl
} // namespace widgets
} // namespace ui
} // namespace cyberverse
