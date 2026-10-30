/**
 * @file text_input.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets text input implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/impl/text_input.hpp"

#include <algorithm>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "cyberverse/ui/widgets/detail/font.hpp"
#include "cyberverse/ui/widgets/event.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

size text_input::measure(const state& s, const theme& th, const constraints& /*lim*/) const noexcept {
  const text_input_style& style = style_override ? *style_override : th.text_input;
  const detail::font_scope font(detail::resolve_font_size(s, th));
  const char* shown = value != nullptr && !value->empty() ? value->c_str() : placeholder.c_str();
  const ImVec2 text_size = ImGui::CalcTextSize(shown);
  const float frame_height = ImGui::GetFrameHeight();

  float width = text_size.x + 2.0f * style.padding_x;
  float height = std::max(frame_height, text_size.y + 2.0f * style.padding_y);
  if (s.has(property_type::width)) {
    width = std::max(0.0f, s.value(property_type::width));
  }
  if (s.has(property_type::height)) {
    height = std::max(0.0f, s.value(property_type::height));
  }
  return size{ std::max(0.0f, width), std::max(0.0f, height) };
}

void text_input::draw(state& s, draw_context& ctx) const {
  const rect& bounds = s.bounds();
  if (value == nullptr || bounds.width <= 0.0f || bounds.height <= 0.0f) {
    return;
  }

  const text_input_style& style = style_override ? *style_override : ctx.appearance.text_input;
  const detail::font_scope font(detail::resolve_font_size(s, ctx.appearance));

  ImGui::PushID(s.id().data(), s.id().data() + s.id().size());
  ImGui::PushStyleColor(ImGuiCol_FrameBg, detail::to_u32(style.background));
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, detail::to_u32(style.background_hovered));
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, detail::to_u32(style.background_active));
  ImGui::PushStyleColor(ImGuiCol_Border, detail::to_u32(style.border));
  ImGui::PushStyleColor(ImGuiCol_Text, detail::to_u32(style.text));
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.rounding);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.border_width);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ style.padding_x, style.padding_y });
  ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(ctx.opacity, 0.0f, 1.0f));

  ImGui::SetCursorScreenPos(ImVec2{ bounds.x, bounds.y });
  ImGui::SetNextItemWidth(bounds.width);
  ImGuiInputTextFlags flags = 0;
  if (password) {
    flags |= ImGuiInputTextFlags_Password;
  }
  if (read_only) {
    flags |= ImGuiInputTextFlags_ReadOnly;
  }
  const bool edited = ImGui::InputTextWithHint("##widgets_text_input", placeholder.c_str(), value, flags);

  ImGui::PopStyleVar(4);
  ImGui::PopStyleColor(5);
  ImGui::PopID();

  if (edited) {
    s.emit(event_type::value_changed);
  }
}

} // namespace impl
} // namespace widgets
} // namespace ui
} // namespace cyberverse
