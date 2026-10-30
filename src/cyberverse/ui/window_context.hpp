/**
 * @file window_context.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Per-window frame context
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>

#include "cyberverse/ui/window.hpp"

namespace cyberverse {
namespace ui {

class host;

/**
 * @brief Frame information handed to window::draw()
 *
 * The context is valid only for the duration of the draw() call and must not be
 * stored. bounds is the logical viewport of the host; the managed window
 * covers exactly that rectangle.
 *
 */
struct window_context {
  const host& ui;                ///< Owning UI runtime
  rect bounds{};                 ///< Logical viewport rectangle
  double delta_time = 0.0;       ///< Logical frame delta in seconds; 0 when the clock is paused
  std::uint64_t frame_index = 0; ///< Monotonic UI frame index
  float scale = 1.0f;            ///< Logical-to-physical pixel scale
};

} // namespace ui
} // namespace cyberverse
