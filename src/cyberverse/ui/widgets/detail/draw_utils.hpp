/**
 * @file draw_utils.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Small drawing helpers shared by widgets
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "cyberverse/ui/panel.hpp"
#include "cyberverse/ui/widgets/detail/style_resolver.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

[[nodiscard]] inline ImVec2 rect_min(const rect& value) noexcept { return ImVec2{ value.x, value.y }; }

[[nodiscard]] inline ImVec2 rect_max(const rect& value) noexcept {
  return ImVec2{ value.x + value.width, value.y + value.height };
}

[[nodiscard]] inline ImU32 to_u32(const color& value) noexcept {
  return ImGui::ColorConvertFloat4ToU32(ImVec4{ value.r, value.g, value.b, value.a });
}

inline void fill_rect(ImDrawList* draw_list, const rect& bounds, ImU32 color, float rounding) noexcept {
  if (draw_list == nullptr || bounds.width <= 0.0f || bounds.height <= 0.0f) {
    return;
  }
  draw_list->AddRectFilled(rect_min(bounds), rect_max(bounds), color, std::max(0.0f, rounding));
}

inline void
stroke_rect(ImDrawList* draw_list, const rect& bounds, ImU32 color, float thickness, float rounding) noexcept {
  if (draw_list == nullptr || bounds.width <= 0.0f || bounds.height <= 0.0f || thickness <= 0.0f) {
    return;
  }
  draw_list->AddRect(rect_min(bounds), rect_max(bounds), color, std::max(0.0f, rounding), ImDrawFlags_None, thickness);
}

inline void draw_container(ImDrawList* draw_list,
                           const rect& bounds,
                           const resolved_style& style,
                           bool hovered = false,
                           bool active = false) noexcept {
  const ImU32 background = active ? style.background_active : (hovered ? style.background_hovered : style.background);
  if (style.has_background) {
    fill_rect(draw_list, bounds, background, style.radius);
  }
  if (style.border_width > 0.0f) {
    stroke_rect(draw_list, bounds, style.border, style.border_width, style.radius);
  }
}

inline void clip_rect(ImDrawList* draw_list, const rect& bounds) noexcept {
  if (draw_list != nullptr) {
    draw_list->PushClipRect(rect_min(bounds), rect_max(bounds), true);
  }
}

inline void unclip_rect(ImDrawList* draw_list) noexcept {
  if (draw_list != nullptr) {
    draw_list->PopClipRect();
  }
}

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
