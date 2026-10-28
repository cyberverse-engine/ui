/**
 * @file button.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native ImGui button adapter implementation
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/button.hpp"

#include <algorithm>
#include <utility>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/layout_types.hpp"
#include "cyberverse/ui/widgets/detail/style_resolver.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

button::button(std::string label, std::function<void()> on_click)
    : label_{ std::move(label) }, on_click_{ std::move(on_click) } {
  id_ = label_;
}

detail::measured_size button::measure(detail::measure_context& context, const detail::constraints& /*limits*/) {
  const theme& appearance = detail::appearance_or_default(context.appearance);
  const detail::resolved_style style = detail::resolve_style(detail::style_kind::button, appearance, style_override_);
  const ImVec2 text_size = ImGui::CalcTextSize(label_.c_str());
  return detail::measured_size{ text_size.x + 2.0f * style.padding_x,
                                std::max(text_size.y + 2.0f * style.padding_y, ImGui::GetFrameHeight()) };
}

void button::draw(detail::draw_context& context) {
  if (bounds_.width <= 0.0f || bounds_.height <= 0.0f) {
    return;
  }

  const detail::resolved_style style =
      detail::resolve_style(detail::style_kind::button, context.appearance, style_override_);

  ImGui::PushStyleColor(ImGuiCol_Button, style.background);
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, style.background_hovered);
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, style.background_active);
  ImGui::PushStyleColor(ImGuiCol_Border, style.border);
  ImGui::PushStyleColor(ImGuiCol_Text, style.text);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.radius);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.border_width);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ style.padding_x, style.padding_y });

  ImGui::SetCursorScreenPos(ImVec2{ bounds_.x, bounds_.y });
  const bool clicked =
      ImGui::Button(label_.c_str(), ImVec2{ std::max(1.0f, bounds_.width), std::max(1.0f, bounds_.height) });

  ImGui::PopStyleVar(3);
  ImGui::PopStyleColor(5);

  if (clicked && on_click_) {
    on_click_();
  }
}

void button::visit_children(const child_visitor& /*visitor*/) const {}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
