/**
 * @file scroll_canvas.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Internal scroll viewport/scrollbar implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/detail/scroll_canvas.hpp"

#include <algorithm>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/font.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

namespace {

[[nodiscard]] float read_scroll(const state& s, property_type property, float maximum) noexcept {
  const float value = s.has(property) ? s.value(property) : 0.0f;
  return std::clamp(value, 0.0f, maximum);
}

void write_scroll(state& s, property_type property, float value, float maximum) noexcept {
  const float clamped = std::clamp(value, 0.0f, maximum);
  if (!s.has(property) || s.value(property) != clamped) {
    s.set(property, clamped);
  }
}

} // namespace

scroll_canvas scroll_canvas::begin(state& owner,
                                   const scroll_view_style& style,
                                   const rect& bounds,
                                   const size& content,
                                   const scroll_options& options,
                                   float opacity) {
  scroll_canvas result;
  result.owner_ = &owner;
  result.style_ = &style;
  result.bounds_ = bounds;
  result.content_ = content;
  result.opacity_ = opacity;
  result.max_scroll_x_ = std::max(0.0f, content.width - bounds.width);
  result.max_scroll_y_ = std::max(0.0f, content.height - bounds.height);
  result.scroll_x = read_scroll(owner, property_type::scroll_x, result.max_scroll_x_);
  result.scroll_y = read_scroll(owner, property_type::scroll_y, result.max_scroll_y_);
  write_scroll(owner, property_type::scroll_x, result.scroll_x, result.max_scroll_x_);
  write_scroll(owner, property_type::scroll_y, result.scroll_y, result.max_scroll_y_);

  if (ImGui::IsMouseHoveringRect(
          ImVec2{ bounds.x, bounds.y }, ImVec2{ bounds.x + bounds.width, bounds.y + bounds.height }, true)) {
    const ImGuiIO& io = ImGui::GetIO();
    float wheel_x = io.MouseWheelH;
    float wheel_y = io.MouseWheel;
    if (io.KeyShift) {
      wheel_x += wheel_y;
      wheel_y = 0.0f;
    }
    const float step = std::max(0.0f, options.wheel_step) * ImGui::GetTextLineHeight();
    result.scroll_x = std::clamp(result.scroll_x - wheel_x * step, 0.0f, result.max_scroll_x_);
    result.scroll_y = std::clamp(result.scroll_y - wheel_y * step, 0.0f, result.max_scroll_y_);
    write_scroll(owner, property_type::scroll_x, result.scroll_x, result.max_scroll_x_);
    write_scroll(owner, property_type::scroll_y, result.scroll_y, result.max_scroll_y_);
  }
  return result;
}

void scroll_canvas::draw_background() const {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 min{ bounds_.x, bounds_.y };
  const ImVec2 max{ bounds_.x + bounds_.width, bounds_.y + bounds_.height };
  draw_list->AddRectFilled(min, max, to_u32(style_->background, opacity_), style_->rounding);
  if (style_->border_width > 0.0f) {
    draw_list->AddRect(min, max, to_u32(style_->border, opacity_), style_->rounding, 0, style_->border_width);
  }
}

void scroll_canvas::push_id() const { ImGui::PushID(owner_->id().data(), owner_->id().data() + owner_->id().size()); }

void scroll_canvas::pop_id() const { ImGui::PopID(); }

void scroll_canvas::push_clip() const {
  ImGui::PushClipRect(
      ImVec2{ bounds_.x, bounds_.y }, ImVec2{ bounds_.x + bounds_.width, bounds_.y + bounds_.height }, true);
}

void scroll_canvas::pop_clip() const { ImGui::PopClipRect(); }

void scroll_canvas::draw_scrollbars() const {
  const bool show_vertical = content_.height > bounds_.height;
  const bool show_horizontal = content_.width > bounds_.width;
  if (!show_vertical && !show_horizontal) {
    return;
  }

  const ImGuiStyle& im_style = ImGui::GetStyle();
  const float bar_size = std::max(4.0f, im_style.ScrollbarSize);
  const float min_thumb = std::max(8.0f, bar_size);
  const float alpha = std::clamp(opacity_, 0.0f, 1.0f);
  const ImU32 track_color = ImGui::GetColorU32(ImGuiCol_ScrollbarBg, alpha);
  ImDrawList* draw_list = ImGui::GetWindowDrawList();

  if (show_vertical) {
    const float track_height = std::max(0.0f, bounds_.height - (show_horizontal ? bar_size : 0.0f));
    const float track_x = bounds_.x + bounds_.width - bar_size;
    const float track_y = bounds_.y;
    const float thumb_height =
        std::min(track_height, std::max(min_thumb, track_height * bounds_.height / std::max(1.0f, content_.height)));
    const float travel = std::max(0.0f, track_height - thumb_height);
    const float thumb_y = track_y + (max_scroll_y_ > 0.0f ? travel * (scroll_y / max_scroll_y_) : 0.0f);

    ImGui::SetCursorScreenPos(ImVec2{ track_x, track_y });
    ImGui::InvisibleButton("##scroll_view_vertical", ImVec2{ bar_size, track_height });
    const bool active = ImGui::IsItemActive();
    const bool hovered = ImGui::IsItemHovered();
    if (active && travel > 0.0f) {
      const float mouse = ImGui::GetIO().MousePos.y - track_y - thumb_height * 0.5f;
      const float ratio = std::clamp(mouse / travel, 0.0f, 1.0f);
      write_scroll(*owner_, property_type::scroll_y, ratio * max_scroll_y_, max_scroll_y_);
    }

    draw_list->AddRectFilled(
        ImVec2{ track_x, track_y }, ImVec2{ track_x + bar_size, track_y + track_height }, track_color, 2.0f);
    draw_list->AddRectFilled(
        ImVec2{ track_x, thumb_y },
        ImVec2{ track_x + bar_size, thumb_y + thumb_height },
        ImGui::GetColorU32(active ? ImGuiCol_ScrollbarGrabActive
                                  : (hovered ? ImGuiCol_ScrollbarGrabHovered : ImGuiCol_ScrollbarGrab),
                           alpha),
        2.0f);
  }

  if (show_horizontal) {
    const float track_width = std::max(0.0f, bounds_.width - (show_vertical ? bar_size : 0.0f));
    const float track_x = bounds_.x;
    const float track_y = bounds_.y + bounds_.height - bar_size;
    const float thumb_width =
        std::min(track_width, std::max(min_thumb, track_width * bounds_.width / std::max(1.0f, content_.width)));
    const float travel = std::max(0.0f, track_width - thumb_width);
    const float thumb_x = track_x + (max_scroll_x_ > 0.0f ? travel * (scroll_x / max_scroll_x_) : 0.0f);

    ImGui::SetCursorScreenPos(ImVec2{ track_x, track_y });
    ImGui::InvisibleButton("##scroll_view_horizontal", ImVec2{ track_width, bar_size });
    const bool active = ImGui::IsItemActive();
    const bool hovered = ImGui::IsItemHovered();
    if (active && travel > 0.0f) {
      const float mouse = ImGui::GetIO().MousePos.x - track_x - thumb_width * 0.5f;
      const float ratio = std::clamp(mouse / travel, 0.0f, 1.0f);
      write_scroll(*owner_, property_type::scroll_x, ratio * max_scroll_x_, max_scroll_x_);
    }

    draw_list->AddRectFilled(
        ImVec2{ track_x, track_y }, ImVec2{ track_x + track_width, track_y + bar_size }, track_color, 2.0f);
    draw_list->AddRectFilled(
        ImVec2{ thumb_x, track_y },
        ImVec2{ thumb_x + thumb_width, track_y + bar_size },
        ImGui::GetColorU32(active ? ImGuiCol_ScrollbarGrabActive
                                  : (hovered ? ImGuiCol_ScrollbarGrabHovered : ImGuiCol_ScrollbarGrab),
                           alpha),
        2.0f);
  }
}

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
