/**
 * @file scroll_view_types.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Scroll view public configuration
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets/stack_types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Primary content flow direction
 *
 */
enum class scroll_content_direction { vertical, horizontal };

/**
 * @brief Scroll view configuration
 *
 * Scrolling itself is always available on both axes. The direction only
 * determines how the child components are packed.
 *
 */
struct scroll_options {
  scroll_content_direction direction = scroll_content_direction::vertical;
  float gap = 0.0f;
  insets padding{};
  float wheel_step = 3.0f;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
