/**
 * @file box.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Box implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <optional>

#include "cyberverse/ui/widgets/box_types.hpp"
#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/context.hpp"
#include "cyberverse/ui/widgets/detail/box_canvas.hpp"
#include "cyberverse/ui/widgets/detail/state_access.hpp"
#include "cyberverse/ui/widgets/rect.hpp"
#include "cyberverse/ui/widgets/state.hpp"
#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

/**
 * @brief Styled single-child container
 *
 */
template <typename Child>
struct box {
  box_options options;
  std::optional<box_style> style_override;
  Child& child;

  [[nodiscard]] size measure(const state& s, const theme& th, const constraints& lim) const noexcept {
    const size inner = child.measure(th, lim);
    float width = inner.width + options.padding.left + options.padding.right;
    float height = inner.height + options.padding.top + options.padding.bottom;
    if (s.has(property_type::width)) {
      width = std::max(0.0f, s.value(property_type::width));
    }
    if (s.has(property_type::height)) {
      height = std::max(0.0f, s.value(property_type::height));
    }
    return size{ std::max(0.0f, width), std::max(0.0f, height) };
  }

  void draw(state& s, draw_context& ctx) const {
    const rect& bounds = s.bounds();
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) {
      return;
    }

    const box_style& style = style_override ? *style_override : ctx.appearance.box;
    detail::draw_box_background(bounds, style, ctx.opacity);

    const float child_width = std::max(0.0f, bounds.width - options.padding.left - options.padding.right);
    const float child_height = std::max(0.0f, bounds.height - options.padding.top - options.padding.bottom);
    detail::state_access::set_bounds(
        child.s, rect{ bounds.x + options.padding.left, bounds.y + options.padding.top, child_width, child_height });

    detail::push_box_clip(bounds);
    child.draw(ctx);
    detail::pop_box_clip();
  }

  template <typename F>
  void visit_states(F&& f) const {
    child.visit_states(f);
  }
};

} // namespace impl

} // namespace widgets
} // namespace ui
} // namespace cyberverse
