/**
 * @file checkbox.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native ImGui checkbox adapter implementation
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/checkbox.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/layout_types.hpp"
#include "cyberverse/ui/widgets/detail/style_resolver.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

checkbox::checkbox(std::string label, bool& value) : label_{ std::move(label) }, value_{ &value } {
  if (label_.empty()) {
    throw std::invalid_argument("cyberverse::ui::widgets::checkbox: label must not be empty");
  }
  id_ = label_;
}

detail::measured_size checkbox::measure(detail::measure_context& context, const detail::constraints& /*limits*/) {
  const theme& appearance = detail::appearance_or_default(context.appearance);
  const detail::resolved_style style = detail::resolve_style(detail::style_kind::checkbox, appearance, style_override_);
  const float frame_height = ImGui::GetFrameHeight();
  const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
  const ImVec2 text_size = ImGui::CalcTextSize(label_.c_str());
  return detail::measured_size{ frame_height + spacing + text_size.x + 2.0f * style.padding_x,
                                std::max(frame_height, text_size.y) + 2.0f * style.padding_y };
}

void checkbox::draw(detail::draw_context& context) {
  if (value_ == nullptr || bounds_.width <= 0.0f || bounds_.height <= 0.0f) {
    return;
  }

  const detail::resolved_style style =
      detail::resolve_style(detail::style_kind::checkbox, context.appearance, style_override_);

  ImGui::PushStyleColor(ImGuiCol_FrameBg, style.background);
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, style.background_hovered);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, style.background_active);
  ImGui::PushStyleColor(ImGuiCol_Border, style.border);
  ImGui::PushStyleColor(ImGuiCol_Text, style.text);
  ImGui::PushStyleColor(ImGuiCol_CheckMark, style.focus_ring);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.radius);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.border_width);

  const float frame_height = ImGui::GetFrameHeight();
  const float y = bounds_.y + std::max(0.0f, (bounds_.height - frame_height) * 0.5f);
  ImGui::SetCursorScreenPos(ImVec2{ bounds_.x, y });
  ImGui::Checkbox(label_.c_str(), value_);

  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor(6);
}

void checkbox::visit_children(const child_visitor& /*visitor*/) const {}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
