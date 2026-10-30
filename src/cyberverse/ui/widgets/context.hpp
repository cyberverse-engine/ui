/**
 * @file context.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Per-frame widget context
 * @version 0.2
 * @date 2026-10-31
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets/theme.hpp"
#include "cyberverse/ui/window_context.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Intrinsic measurement constraints
 *
 */
struct constraints {
  float available_width = 0.0f;
  float available_height = 0.0f;
};

/**
 * @brief Per-frame input and draw state shared by widget components
 *
 * The context is valid only for the duration of widgets::run(). window provides
 * the frame delta that drives animation; appearance is the resolved theme;
 * opacity is the inherited draw alpha, multiplied by each component's opacity.
 *
 */
struct widget_context {
  window_context& window;
  const theme& appearance;
  float opacity = 1.0f;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
