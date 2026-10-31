/**
 * @file native.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native ImGui component implementation
 * @version 0.1
 * @date 2026-10-31
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <utility>

#include "cyberverse/ui/widgets/context.hpp"
#include "cyberverse/ui/widgets/state.hpp"
#include "cyberverse/ui/widgets/theme.hpp"
#include "cyberverse/ui/widgets/types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

/**
 * @brief Component wrapper around caller-provided measure and draw callables
 *
 */
template <typename Measure, typename Draw>
struct native {
  mutable Measure measure_fn;
  mutable Draw draw_fn;

  native(Measure measure_value, Draw draw_value)
      : measure_fn(std::move(measure_value)), draw_fn(std::move(draw_value)) {}

  [[nodiscard]] layout_size measure(const state& s, const theme& th, const constraints& lim) const {
    layout_size result = measure_fn(th, lim);
    result.width = std::max(0.0f, result.width);
    result.height = std::max(0.0f, result.height);
    if (s.has(property_type::width)) {
      result.width = std::max(0.0f, s.value(property_type::width));
    }
    if (s.has(property_type::height)) {
      result.height = std::max(0.0f, s.value(property_type::height));
    }
    return result;
  }

  void draw(state& s, widget_context& ctx) const {
    const rect& bounds = s.bounds();
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) {
      return;
    }
    draw_fn(ctx, bounds);
  }

  template <typename F>
  void visit_states(F&& /*visitor*/) const noexcept {}
};

} // namespace impl
} // namespace widgets
} // namespace ui
} // namespace cyberverse
