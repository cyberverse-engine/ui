/**
 * @file scroll_canvas.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Internal scroll viewport/scrollbar bridge
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets2/context.hpp"
#include "cyberverse/ui/widgets2/rect.hpp"
#include "cyberverse/ui/widgets2/scroll_view_types.hpp"
#include "cyberverse/ui/widgets2/state.hpp"
#include "cyberverse/ui/widgets2/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {
namespace detail {

/**
 * @brief Per-frame scroll state and viewport drawing operations
 *
 * Keeps Dear ImGui out of the scroll_view implementation header.
 *
 */
struct scroll_canvas {
  float scroll_x = 0.0f;
  float scroll_y = 0.0f;

  static scroll_canvas begin(state& owner,
                             const scroll_view_style& style,
                             const rect& bounds,
                             const size& content,
                             const scroll_options& options,
                             float opacity);

  void draw_background() const;
  void push_id() const;
  void pop_id() const;
  void push_clip() const;
  void pop_clip() const;
  void draw_scrollbars() const;

private:
  state* owner_ = nullptr;
  const scroll_view_style* style_ = nullptr;
  rect bounds_{};
  size content_{};
  float opacity_ = 1.0f;
  float max_scroll_x_ = 0.0f;
  float max_scroll_y_ = 0.0f;
};

} // namespace detail
} // namespace widgets2
} // namespace ui
} // namespace cyberverse
