/**
 * @file box_canvas.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Internal box background/clip bridge
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets/theme.hpp"
#include "cyberverse/ui/widgets/types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

void draw_box_background(const rect& bounds, const box_style& style, float opacity);
void push_box_clip(const rect& bounds);
void pop_box_clip();

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
