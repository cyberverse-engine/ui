/**
 * @file panel_context.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Per-panel frame context
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>

#include "cyberverse/ui/panel.hpp"

namespace cyberverse {
namespace ui {

class layer;
class system;

/**
 * @brief Frame information handed to panel::draw()
 *
 * The context is valid only for the duration of the draw() call and must not be
 * stored. bounds is the logical viewport of the layer; the managed window
 * covers exactly that rectangle.
 *
 */
struct panel_context {
  const system& ui;              ///< Owning UI runtime
  const layer& stage;            ///< Layer that owns the panel
  rect bounds{};                 ///< Logical layer viewport rectangle
  double delta_time = 0.0;       ///< Frame delta in seconds
  std::uint64_t frame_index = 0; ///< Monotonic UI frame index
  float scale = 1.0f;            ///< Logical-to-physical pixel scale
};

} // namespace ui
} // namespace cyberverse
