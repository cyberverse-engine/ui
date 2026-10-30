/**
 * @file flex_types.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Flexbox public configuration types
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
 * @brief Flexbox main-axis direction
 *
 */
enum class flex_direction { row, column };

/**
 * @brief Main-axis distribution policy
 *
 */
enum class justify_content { start, center, end, space_between };

/**
 * @brief Cross-axis alignment policy
 *
 */
enum class align_items { start, center, end, stretch };

/**
 * @brief Flexbox layout configuration
 *
 */
struct flex_options {
  flex_direction direction = flex_direction::row;
  float gap = 0.0f;
  insets padding{};
  justify_content justify = justify_content::start;
  align_items align = align_items::start;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
