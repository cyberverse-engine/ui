/**
 * @file box_canvas.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Internal box background/clip implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/detail/box_canvas.hpp"

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/font.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

void draw_box_background(const rect& bounds, const box_style& style, float opacity) {
  ImDrawList* draw_list = ImGui::GetWindowDrawList();
  const ImVec2 min{ bounds.x, bounds.y };
  const ImVec2 max{ bounds.x + bounds.width, bounds.y + bounds.height };
  draw_list->AddRectFilled(min, max, to_u32(style.background, opacity), style.rounding);
  if (style.border_width > 0.0f) {
    draw_list->AddRect(min, max, to_u32(style.border, opacity), style.rounding, 0, style.border_width);
  }
}

void push_box_clip(const rect& bounds) {
  ImGui::PushClipRect(ImVec2{ bounds.x, bounds.y }, ImVec2{ bounds.x + bounds.width, bounds.y + bounds.height }, true);
}

void pop_box_clip() { ImGui::PopClipRect(); }

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
