/**
 * @file scroll_view.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Vertically scrollable flexbox container implementation
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/scroll_view.hpp"

#include <algorithm>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/layout_engine.hpp"
#include "cyberverse/ui/widgets/detail/layout_types.hpp"
#include "cyberverse/ui/widgets/detail/style_resolver.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

scroll_view::scroll_view() : content_{ flex_direction::column } {}

scroll_view& scroll_view::direction(flex_direction value) {
  content_.direction(value);
  return *this;
}

scroll_view& scroll_view::gap(float value) {
  content_.gap(value);
  return *this;
}

scroll_view& scroll_view::padding(insets value) {
  padding_ = value;
  return *this;
}

detail::measured_size scroll_view::measure(detail::measure_context& context, const detail::constraints& limits) {
  const theme& appearance = detail::appearance_or_default(context.appearance);
  const detail::resolved_style style =
      detail::resolve_style(detail::style_kind::scroll_view, appearance, style_override_);

  const float horizontal_padding = std::max(0.0f, padding_.left) + std::max(0.0f, padding_.right);
  const float vertical_padding = std::max(0.0f, padding_.top) + std::max(0.0f, padding_.bottom);

  detail::constraints content_limits;
  content_limits.width_mode = limits.width_mode;
  content_limits.height_mode = detail::measure_mode::undefined;
  content_limits.available_width = std::max(0.0f, limits.available_width - horizontal_padding);
  content_limits.available_height = 0.0f;

  const detail::measured_size content_size = detail::measure(content_, content_limits, appearance);
  return detail::measured_size{ content_size.width + horizontal_padding + 2.0f * style.padding_x,
                                content_size.height + vertical_padding + 2.0f * style.padding_y };
}

void scroll_view::draw(detail::draw_context& context) {
  if (bounds_.width <= 0.0f || bounds_.height <= 0.0f) {
    return;
  }

  const detail::resolved_style style =
      detail::resolve_style(detail::style_kind::scroll_view, context.appearance, style_override_);

  ImGui::PushStyleColor(ImGuiCol_ChildBg, style.background);
  ImGui::PushStyleColor(ImGuiCol_Border, style.border);
  ImGui::PushStyleColor(ImGuiCol_Text, style.text);
  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, style.radius);
  ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, style.border_width);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0.0f, 0.0f });

  const std::string child_id = id_.empty() ? std::string{ "##scroll_view" } : "##" + id_;
  ImGui::SetCursorScreenPos(ImVec2{ bounds_.x, bounds_.y });
  const bool visible = ImGui::BeginChild(child_id.c_str(),
                                         ImVec2{ std::max(1.0f, bounds_.width), std::max(1.0f, bounds_.height) },
                                         ImGuiChildFlags_Borders,
                                         ImGuiWindowFlags_None);

  if (visible) {
    ImGui::SetCursorPos(ImVec2{ std::max(0.0f, padding_.left), std::max(0.0f, padding_.top) });
    const ImVec2 content_origin = ImGui::GetCursorScreenPos();
    const ImVec2 content_available = ImGui::GetContentRegionAvail();
    if (content_available.x > 0.0f && content_available.y > 0.0f) {
      detail::render(content_, content_origin, content_available, context.appearance);
    }
  }

  ImGui::EndChild();
  ImGui::PopStyleVar(3);
  ImGui::PopStyleColor(3);
}

void scroll_view::visit_children(const child_visitor& /*visitor*/) const {}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
