/**
 * @file context.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Per-frame update and draw contexts
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets2/animator.hpp"
#include "cyberverse/ui/widgets2/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {

/**
 * @brief Intrinsic measurement constraints
 *
 */
struct constraints {
  float available_width = 0.0f;
  float available_height = 0.0f;
};

/**
 * @brief State update input: frame delta and shared animation policy
 *
 */
struct update_context {
  float dt = 0.0f;
  animator& animations;
};

/**
 * @brief Draw-only context; never advances animation time
 *
 */
struct draw_context {
  const theme& appearance;
  float opacity = 1.0f;
};

/**
 * @brief Input for the fixed per-frame entry point
 *
 */
struct frame_context {
  float dt = 0.0f;
  const theme& appearance;
  animator& animations;
};

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
