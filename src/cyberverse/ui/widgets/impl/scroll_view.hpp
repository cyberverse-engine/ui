/**
 * @file scroll_view.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Scroll view implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <optional>
#include <tuple>

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/context.hpp"
#include "cyberverse/ui/widgets/detail/scroll_canvas.hpp"
#include "cyberverse/ui/widgets/detail/state_access.hpp"
#include "cyberverse/ui/widgets/scroll_view_types.hpp"
#include "cyberverse/ui/widgets/state.hpp"
#include "cyberverse/ui/widgets/theme.hpp"
#include "cyberverse/ui/widgets/types.hpp"
#include "cyberverse/ui/widgets/visibility.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

/**
 * @brief Dual-axis scroll view holding references to components
 *
 */
template <typename... Ts>
struct scroll_view {
  static constexpr std::size_t child_count = sizeof...(Ts);

  scroll_options options;
  std::optional<scroll_view_style> style_override;
  std::tuple<Ts&...> children;

  scroll_view(scroll_options options_, std::optional<scroll_view_style> style_, std::tuple<Ts&...> children_)
      : options(options_), style_override(std::move(style_)), children(children_) {}

  struct item {
    bool active = false;
    layout_size measured{};
    float x = 0.0f;
    float y = 0.0f;
  };

  template <typename Child>
  [[nodiscard]] item make_item(Child& child, const theme& th, const constraints& lim) const noexcept {
    item result{};
    result.active = child.visibility_value() != visibility::collapsed && !child.s.has_zero_dimension();
    if (result.active) {
      result.measured = child.measure(th, lim);
    }
    return result;
  }

  [[nodiscard]] std::array<item, child_count> layout_items(const theme& th, const constraints& lim) const noexcept {
    std::array<item, child_count> items{};
    std::size_t index = 0;
    std::apply(
        [&](auto&... child) {
          ((items[index] = make_item(child, th, lim), ++index), ...);
        },
        children);

    const bool horizontal = options.direction == scroll_content_direction::horizontal;
    const float gap = std::max(0.0f, options.gap);
    float cursor = 0.0f;
    std::size_t active_count = 0;

    for (item& value : items) {
      if (!value.active) {
        continue;
      }
      if (active_count > 0) {
        cursor += gap;
      }
      if (horizontal) {
        value.x = options.padding.left + cursor;
        value.y = options.padding.top;
        cursor += value.measured.width;
      } else {
        value.x = options.padding.left;
        value.y = options.padding.top + cursor;
        cursor += value.measured.height;
      }
      ++active_count;
    }
    return items;
  }

  [[nodiscard]] layout_size content_size(const std::array<item, child_count>& items) const noexcept {
    const bool horizontal = options.direction == scroll_content_direction::horizontal;
    float width = options.padding.left + options.padding.right;
    float height = options.padding.top + options.padding.bottom;

    for (const item& value : items) {
      if (!value.active) {
        continue;
      }
      width = std::max(width, value.x + value.measured.width + options.padding.right);
      height = std::max(height, value.y + value.measured.height + options.padding.bottom);
    }
    return layout_size{ std::max(0.0f, width), std::max(0.0f, height) };
  }

  [[nodiscard]] layout_size measure(const state& s, const theme& th, const constraints& lim) const noexcept {
    const layout_size content = content_size(layout_items(th, lim));
    layout_size result = content;
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

    const std::array<item, child_count> items =
        layout_items(ctx.appearance, constraints{ bounds.width, bounds.height });
    const layout_size content = content_size(items);
    const scroll_view_style& style = style_override ? *style_override : ctx.appearance.scroll_view;
    const detail::scroll_canvas canvas = detail::scroll_canvas::begin(s, style, bounds, content, options, ctx.opacity);

    canvas.draw_background();
    canvas.push_id();
    canvas.push_clip();
    std::size_t index = 0;
    std::apply(
        [&](auto&... child) {
          ((draw_item(child, items[index], bounds, canvas.scroll_x, canvas.scroll_y, ctx), ++index), ...);
        },
        children);
    canvas.pop_clip();
    canvas.draw_scrollbars();
    canvas.pop_id();
  }

  template <typename F>
  void visit_states(F&& f) const {
    std::apply(
        [&](auto&... child) {
          (child.visit_states(f), ...);
        },
        children);
  }

private:
  template <typename Child>
  static void
  draw_item(Child& child, const item& value, const rect& bounds, float scroll_x, float scroll_y, widget_context& ctx) {
    if (!value.active) {
      detail::state_access::set_bounds(child.s, rect{});
      return;
    }

    const rect child_bounds{
      bounds.x + value.x - scroll_x, bounds.y + value.y - scroll_y, value.measured.width, value.measured.height
    };
    detail::state_access::set_bounds(child.s, child_bounds);
    child.draw(ctx);
  }
};

} // namespace impl

template <component_like... Ts>
[[nodiscard]] component<impl::scroll_view<Ts...>>
scroll_view(state& s, scroll_options options, std::optional<scroll_view_style> style, Ts&... children) {
  static_assert((component_like<Ts> && ...), "scroll_view children must satisfy component_like");
  return component<impl::scroll_view<Ts...>>{
    s, impl::scroll_view<Ts...>{ options, std::move(style), std::tie(children...) }
  };
}

template <component_like... Ts>
[[nodiscard]] component<impl::scroll_view<Ts...>> scroll_view(state& s, scroll_options options, Ts&... children) {
  return scroll_view(s, options, std::nullopt, children...);
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
