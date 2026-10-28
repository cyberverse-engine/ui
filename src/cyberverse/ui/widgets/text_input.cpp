/**
 * @file text_input.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native single-line ImGui text input implementation
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/text_input.hpp"

#include <algorithm>
#include <stdexcept>
#include <utility>

#include <imgui.h>
#include <misc/cpp/imgui_stdlib.h>

#include "cyberverse/ui/widgets/detail/layout_types.hpp"
#include "cyberverse/ui/widgets/detail/style_resolver.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

text_input::text_input(std::string id, std::string& value) : value_{ &value } {
  if (id.empty()) {
    throw std::invalid_argument("cyberverse::ui::widgets::text_input: id must not be empty");
  }
  id_ = std::move(id);
}

text_input& text_input::placeholder(std::string_view value) {
  placeholder_.assign(value.data(), value.size());
  return *this;
}

text_input& text_input::password(bool enabled) {
  password_ = enabled;
  return *this;
}

text_input& text_input::read_only(bool enabled) {
  read_only_ = enabled;
  return *this;
}

text_input& text_input::on_change(std::function<void(std::string_view)> callback) {
  on_change_ = std::move(callback);
  return *this;
}

detail::measured_size text_input::measure(detail::measure_context& context, const detail::constraints& /*limits*/) {
  const theme& appearance = detail::appearance_or_default(context.appearance);
  const detail::resolved_style style =
      detail::resolve_style(detail::style_kind::text_input, appearance, style_override_);

  std::string sample = (value_ != nullptr && !value_->empty()) ? *value_ : placeholder_;
  if (sample.empty()) {
    sample = " ";
  }
  const ImVec2 text_size = ImGui::CalcTextSize(sample.c_str());
  return detail::measured_size{ text_size.x + 2.0f * style.padding_x,
                                std::max(text_size.y + 2.0f * style.padding_y, ImGui::GetFrameHeight()) };
}

void text_input::draw(detail::draw_context& context) {
  if (value_ == nullptr || bounds_.width <= 0.0f || bounds_.height <= 0.0f) {
    return;
  }

  const detail::resolved_style style =
      detail::resolve_style(detail::style_kind::text_input, context.appearance, style_override_);

  ImGui::PushStyleColor(ImGuiCol_FrameBg, style.background);
  ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, style.background_hovered);
  ImGui::PushStyleColor(ImGuiCol_FrameBgActive, style.background_active);
  ImGui::PushStyleColor(ImGuiCol_Border, style.border);
  ImGui::PushStyleColor(ImGuiCol_Text, style.text);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.radius);
  ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.border_width);
  ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ style.padding_x, style.padding_y });

  ImGuiInputTextFlags flags = 0;
  if (password_) {
    flags |= ImGuiInputTextFlags_Password;
  }
  if (read_only_) {
    flags |= ImGuiInputTextFlags_ReadOnly;
  }

  const float frame_height = ImGui::GetFrameHeight();
  const float y = bounds_.y + std::max(0.0f, (bounds_.height - frame_height) * 0.5f);
  ImGui::SetCursorScreenPos(ImVec2{ bounds_.x, y });
  ImGui::SetNextItemWidth(std::max(1.0f, bounds_.width));

  const std::string label = "##" + id_;
  const bool edited = ImGui::InputTextWithHint(label.c_str(), placeholder_.c_str(), value_, flags);

  ImGui::PopStyleVar(3);
  ImGui::PopStyleColor(5);

  if (edited && on_change_) {
    on_change_(*value_);
  }
}

void text_input::visit_children(const child_visitor& /*visitor*/) const {}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
