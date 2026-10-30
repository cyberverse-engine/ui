/**
 * @file stack_types.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Stack public configuration types
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

namespace cyberverse {
namespace ui {
namespace widgets2 {

/**
 * @brief Four-sided logical inset
 *
 */
struct insets {
  float left = 0.0f;
  float top = 0.0f;
  float right = 0.0f;
  float bottom = 0.0f;
};

/**
 * @brief Stack layout direction
 *
 */
enum class stack_direction { horizontal, vertical };

/**
 * @brief Stack layout configuration
 *
 */
struct stack_options {
  stack_direction direction = stack_direction::vertical;
  float gap = 0.0f;
  insets padding{};
};

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
