/**
 * @file collapse_group.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Collapsible titled container implementation
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/collapse_group.hpp"

#include <algorithm>
#include <cmath>
#include <utility>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/draw_utils.hpp"
#include "cyberverse/ui/widgets/detail/layout_engine.hpp"
#include "cyberverse/ui/widgets/detail/layout_types.hpp"
#include "cyberverse/ui/widgets/detail/style_resolver.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

namespace {

[[nodiscard]] float horizontal_padding(const insets& value) noexcept {
  return std::max(0.0f, value.left) + std::max(0.0f, value.right);
}

[[nodiscard]] float vertical_padding(const insets& value) noexcept {
  return std::max(0.0f, value.top) + std::max(0.0f, value.bottom);
}

} // namespace

collapse_group::collapse_group(std::string title, bool& open)
    : title_{ std::move(title) }, open_{ &open }, body_{ flex_direction::column } {
  id_ = title_;
}

collapse_group& collapse_group::gap(float value) {
  gap_ = value;
  return *this;
}

collapse_group& collapse_group::padding(insets value) {
  padding_ = value;
  return *this;
}

detail::measured_size collapse_group::measure(detail::measure_context& context, const detail::constraints& limits) {
  const theme& appearance = detail::appearance_or_default(context.appearance);
  const detail::resolved_style header_style =
      detail::resolve_style(detail::style_kind::collapse_header, appearance, style_override_);
  const ImVec2 title_size = ImGui::CalcTextSize(title_.c_str());
  const float header_height = std::max(ImGui::GetFrameHeight(), title_size.y + 2.0f * header_style.padding_y);
  const float padding = horizontal_padding(padding_);

  float body_width = 0.0f;
  float body_height = 0.0f;
  if (open_ != nullptr && *open_) {
    detail::constraints content_limits;
    content_limits.width_mode = limits.width_mode;
    content_limits.height_mode = detail::measure_mode::undefined;
    content_limits.available_width = std::max(0.0f, limits.available_width - padding);
    content_limits.available_height = 0.0f;

    const detail::measured_size body_size = detail::measure(body_, content_limits, appearance);
    body_width = body_size.width;
    body_height = body_size.height;
  }

  return detail::measured_size{ std::max(title_size.x + 2.0f * header_style.padding_x, body_width) + padding,
                                vertical_padding(padding_) + header_height +
                                    (body_height > 0.0f ? gap_ + body_height : 0.0f) };
}

void collapse_group::draw(detail::draw_context& context) {
  if (bounds_.width <= 0.0f || bounds_.height <= 0.0f || open_ == nullptr) {
    return;
  }

  const detail::resolved_style header_style =
      detail::resolve_style(detail::style_kind::collapse_header, context.appearance, style_override_);
  const detail::resolved_style body_style =
      detail::resolve_style(detail::style_kind::collapse_body, context.appearance, style_override_);

  const float left = std::max(0.0f, padding_.left);
  const float top = std::max(0.0f, padding_.top);
  const float right = std::max(0.0f, padding_.right);
  const float bottom = std::max(0.0f, padding_.bottom);
  const float header_height =
      std::max(ImGui::GetFrameHeight(), ImGui::CalcTextSize(title_.c_str()).y + 2.0f * header_style.padding_y);

  const rect header_rect{
    bounds_.x + left, bounds_.y + top, std::max(0.0f, bounds_.width - left - right), header_height
  };
  detail::draw_container(context.draw_list, header_rect, header_style);

  // Header interaction.
  const std::string header_id = "##" + (id_.empty() ? title_ : id_) + ".header";
  ImGui::SetCursorScreenPos(ImVec2{ header_rect.x, header_rect.y });
  if (ImGui::InvisibleButton(header_id.c_str(),
                             ImVec2{ std::max(1.0f, header_rect.width), std::max(1.0f, header_rect.height) })) {
    *open_ = !*open_;
  }
  const bool hovered = ImGui::IsItemHovered();
  if (hovered) {
    detail::draw_container(context.draw_list, header_rect, header_style, true, false);
  }

  // Triangle and title.
  const float arrow_size = std::max(4.0f, ImGui::GetFontSize() * 0.35f);
  const float center_y = header_rect.y + header_rect.height * 0.5f;
  const float arrow_x = header_rect.x + header_style.padding_x;
  const ImVec2 points[3] = {
    ImVec2{ arrow_x, center_y - arrow_size * 0.5f },
    ImVec2{ arrow_x + arrow_size, center_y },
    ImVec2{ arrow_x, center_y + arrow_size * 0.5f },
  };
  context.draw_list->AddTriangleFilled(points[0], points[1], points[2], header_style.text);

  const ImVec2 text_position{
    header_rect.x + header_style.padding_x + arrow_size + ImGui::GetStyle().ItemInnerSpacing.x,
    header_rect.y + std::max(0.0f, (header_rect.height - ImGui::GetFontSize()) * 0.5f),
  };
  detail::clip_rect(context.draw_list, header_rect);
  context.draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize(), text_position, header_style.text, title_.c_str());
  detail::unclip_rect(context.draw_list);

  if (!*open_) {
    return;
  }

  const float body_y = header_rect.y + header_rect.height + std::max(0.0f, gap_);
  const float body_height = std::max(0.0f, bounds_.y + bounds_.height - bottom - body_y);
  if (body_height <= 0.0f) {
    return;
  }
  const rect body_rect{ bounds_.x + left, body_y, std::max(0.0f, bounds_.width - left - right), body_height };
  detail::draw_container(context.draw_list, body_rect, body_style);

  ImGui::SetCursorScreenPos(ImVec2{ body_rect.x, body_rect.y });
  const ImVec2 body_available = ImGui::GetContentRegionAvail();
  if (body_available.x > 0.0f && body_available.y > 0.0f) {
    detail::render(
        body_, ImVec2{ body_rect.x, body_rect.y }, ImVec2{ body_rect.width, body_height }, context.appearance);
  }
}

void collapse_group::visit_children(const child_visitor& /*visitor*/) const {}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
