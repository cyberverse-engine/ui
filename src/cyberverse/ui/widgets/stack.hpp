/**
 * @file stack.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Auto-sized horizontal or vertical stack
 * @version 0.4
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets/flexbox.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Flow direction of a stack
 *
 */
enum class stack_direction { horizontal, vertical };

/**
 * @brief No-wrap stack whose content determines both dimensions
 *
 * A horizontal stack is laid out as a row and sized to the sum of its children
 * plus gap/padding; its height is the tallest child plus padding. A vertical
 * stack is laid out as a column and sized to the sum of its children plus
 * gap/padding; its width is the widest child plus padding.
 *
 * Both axes default to fit_content. Explicit width/height (including
 * width_stretch/height_stretch) override those defaults through the inherited
 * sizing API. The stack never wraps child placement. If the containing parent is
 * too small, the stack may be constrained and its children may overflow; the
 * stack does not add clipping or wrapping behavior.
 *
 * Children should use fixed or fit-content sizing on the main axis. A
 * main-axis stretch child has no spare space to consume in an intrinsically
 * sized stack; cross-axis stretch and explicit stack sizes remain useful.
 *
 */
class stack final : public flex_container<stack> {
public:
  explicit stack(stack_direction direction = stack_direction::vertical)
      : flex_container<stack>(to_flex_direction(direction)), direction_{ direction } {
    width_fit_content();
    height_fit_content();
  }

  /**
   * @brief Change stack direction without changing explicit size overrides
   *
   */
  stack& direction(stack_direction value) {
    direction_ = value;
    flex_container<stack>::direction(to_flex_direction(value));
    return *this;
  }

  [[nodiscard]] stack_direction direction() const noexcept { return direction_; }

private:
  [[nodiscard]] static flex_direction to_flex_direction(stack_direction value) noexcept {
    return value == stack_direction::horizontal ? flex_direction::row : flex_direction::column;
  }

  stack_direction direction_ = stack_direction::vertical;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
