/**
 * @file flexbox.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Flexbox implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <limits>
#include <tuple>

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/context.hpp"
#include "cyberverse/ui/widgets/detail/state_access.hpp"
#include "cyberverse/ui/widgets/flex_types.hpp"
#include "cyberverse/ui/widgets/state.hpp"
#include "cyberverse/ui/widgets/theme.hpp"
#include "cyberverse/ui/widgets/types.hpp"
#include "cyberverse/ui/widgets/visibility.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

/**
 * @brief No-wrap flexbox holding references to components
 *
 */
template <typename... Ts>
struct flexbox {
  static constexpr std::size_t child_count = sizeof...(Ts);

  flex_options options;
  std::tuple<Ts&...> children;

  flexbox(flex_options options_, std::tuple<Ts&...> children_) : options(options_), children(children_) {}

  struct item {
    bool active = false;
    bool cross_explicit = false;
    float base_main = 0.0f;
    float main = 0.0f;
    float cross = 0.0f;
    float grow = 0.0f;
    float shrink = 1.0f;
    float min_main = 0.0f;
    float max_main = std::numeric_limits<float>::infinity();
    float min_cross = 0.0f;
    float max_cross = std::numeric_limits<float>::infinity();
  };

  template <typename Child>
  [[nodiscard]] item make_item(Child& child, const theme& th, const constraints& lim) const noexcept {
    item result{};
    result.active = child.visibility_value() != visibility::collapsed && !child.s.has_zero_dimension();
    if (!result.active) {
      return result;
    }

    const layout_size measured = child.measure(th, lim);
    const bool horizontal = options.direction == flex_direction::row;
    const property_type main_property = horizontal ? property_type::width : property_type::height;
    const property_type cross_property = horizontal ? property_type::height : property_type::width;

    const float measured_main = horizontal ? measured.width : measured.height;
    const float measured_cross = horizontal ? measured.height : measured.width;

    result.cross_explicit = child.s.has(cross_property);

    result.base_main = child.s.has(property_type::flex_basis)
                           ? std::max(0.0f, child.s.value(property_type::flex_basis))
                           : (child.s.has(main_property) ? std::max(0.0f, child.s.value(main_property))
                                                         : std::max(0.0f, measured_main));
    result.main = result.base_main;
    result.cross =
        result.cross_explicit ? std::max(0.0f, child.s.value(cross_property)) : std::max(0.0f, measured_cross);
    result.grow =
        child.s.has(property_type::flex_grow) ? std::max(0.0f, child.s.value(property_type::flex_grow)) : 0.0f;
    result.shrink =
        child.s.has(property_type::flex_shrink) ? std::max(0.0f, child.s.value(property_type::flex_shrink)) : 1.0f;

    const property_type min_main_property = horizontal ? property_type::min_width : property_type::min_height;
    const property_type max_main_property = horizontal ? property_type::max_width : property_type::max_height;
    const property_type min_cross_property = horizontal ? property_type::min_height : property_type::min_width;
    const property_type max_cross_property = horizontal ? property_type::max_height : property_type::max_width;
    if (child.s.has(min_main_property)) {
      result.min_main = std::max(0.0f, child.s.value(min_main_property));
    }
    if (child.s.has(max_main_property)) {
      result.max_main = std::max(0.0f, child.s.value(max_main_property));
    }
    if (child.s.has(min_cross_property)) {
      result.min_cross = std::max(0.0f, child.s.value(min_cross_property));
    }
    if (child.s.has(max_cross_property)) {
      result.max_cross = std::max(0.0f, child.s.value(max_cross_property));
    }

    const float main_low = std::min(result.min_main, result.max_main);
    const float main_high = std::max(result.min_main, result.max_main);
    const float cross_low = std::min(result.min_cross, result.max_cross);
    const float cross_high = std::max(result.min_cross, result.max_cross);
    result.base_main = std::clamp(result.base_main, main_low, main_high);
    result.main = result.base_main;
    result.cross = std::clamp(result.cross, cross_low, cross_high);
    return result;
  }

  [[nodiscard]] std::array<item, child_count> collect(const theme& th, const constraints& lim) const noexcept {
    std::array<item, child_count> items{};
    std::size_t index = 0;
    std::apply(
        [&](auto&... child) {
          ((items[index] = make_item(child, th, lim), ++index), ...);
        },
        children);
    return items;
  }

  [[nodiscard]] layout_size content_size(const std::array<item, child_count>& items) const noexcept {
    const bool horizontal = options.direction == flex_direction::row;
    float main = 0.0f;
    float cross = 0.0f;
    std::size_t active_count = 0;

    for (const item& value : items) {
      if (!value.active) {
        continue;
      }
      if (active_count > 0) {
        main += std::max(0.0f, options.gap);
      }
      main += value.base_main;
      cross = std::max(cross, value.cross);
      ++active_count;
    }

    main += horizontal ? options.padding.left + options.padding.right : options.padding.top + options.padding.bottom;
    cross += horizontal ? options.padding.top + options.padding.bottom : options.padding.left + options.padding.right;
    return horizontal ? layout_size{ main, cross } : layout_size{ cross, main };
  }

  [[nodiscard]] layout_size measure(const state& s, const theme& th, const constraints& lim) const noexcept {
    const std::array<item, child_count> items = collect(th, lim);
    layout_size result = content_size(items);

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

    std::array<item, child_count> items = collect(ctx.appearance, constraints{ bounds.width, bounds.height });
    const bool horizontal = options.direction == flex_direction::row;
    const float gap = std::max(0.0f, options.gap);
    const float inner_main = std::max(0.0f,
                                      (horizontal ? bounds.width - options.padding.left - options.padding.right
                                                  : bounds.height - options.padding.top - options.padding.bottom));
    const float inner_cross = std::max(0.0f,
                                       (horizontal ? bounds.height - options.padding.top - options.padding.bottom
                                                   : bounds.width - options.padding.left - options.padding.right));

    std::size_t active_count = 0;
    float base_main = 0.0f;
    float total_grow = 0.0f;
    float total_shrink_weight = 0.0f;
    for (const item& value : items) {
      if (!value.active) {
        continue;
      }
      ++active_count;
      base_main += value.base_main;
      total_grow += value.grow;
      total_shrink_weight += value.shrink * value.base_main;
    }

    const float total_gap = active_count > 1 ? gap * static_cast<float>(active_count - 1) : 0.0f;
    const float free = inner_main - total_gap - base_main;

    if (free > 0.0f && total_grow > 0.0f) {
      const float distributable = free;
      for (item& value : items) {
        if (value.active) {
          value.main += distributable * value.grow / total_grow;
        }
      }
    } else if (free < 0.0f && total_shrink_weight > 0.0f) {
      const float deficit = -free;
      for (item& value : items) {
        if (value.active) {
          value.main = std::max(0.0f, value.main - deficit * (value.shrink * value.base_main) / total_shrink_weight);
        }
      }
    }

    float used_main = total_gap;
    for (item& value : items) {
      if (!value.active) {
        continue;
      }
      const float low = std::min(value.min_main, value.max_main);
      const float high = std::max(value.min_main, value.max_main);
      value.main = std::clamp(value.main, low, high);
      used_main += value.main;
    }

    const float leftover = std::max(0.0f, inner_main - used_main);
    float main_cursor = horizontal ? options.padding.left : options.padding.top;
    float justify_extra = 0.0f;
    switch (options.justify) {
    case justify_content::start:
      break;
    case justify_content::center:
      main_cursor += leftover * 0.5f;
      break;
    case justify_content::end:
      main_cursor += leftover;
      break;
    case justify_content::space_between:
      justify_extra = active_count > 1 ? leftover / static_cast<float>(active_count - 1) : 0.0f;
      break;
    }

    std::size_t index = 0;
    std::size_t placed = 0;
    std::apply(
        [&](auto&... child) {
          ((draw_item(child,
                      items[index],
                      horizontal,
                      gap,
                      inner_cross,
                      main_cursor,
                      justify_extra,
                      placed,
                      bounds.x,
                      bounds.y,
                      ctx),
            ++index),
           ...);
        },
        children);
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
  void draw_item(Child& child,
                 const item& value,
                 bool horizontal,
                 float gap,
                 float inner_cross,
                 float& main_cursor,
                 float justify_extra,
                 std::size_t& placed,
                 float origin_x,
                 float origin_y,
                 widget_context& ctx) const {
    if (!value.active) {
      detail::state_access::set_bounds(child.s, rect{});
      return;
    }

    if (placed > 0) {
      main_cursor += gap + justify_extra;
    }

    float cross = value.cross;
    if (options.align == align_items::stretch && !value.cross_explicit) {
      cross = inner_cross;
    }
    const float cross_low = std::min(value.min_cross, value.max_cross);
    const float cross_high = std::max(value.min_cross, value.max_cross);
    cross = std::clamp(cross, cross_low, cross_high);

    float cross_offset = 0.0f;
    switch (options.align) {
    case align_items::start:
    case align_items::stretch:
      cross_offset = 0.0f;
      break;
    case align_items::center:
      cross_offset = std::max(0.0f, (inner_cross - cross) * 0.5f);
      break;
    case align_items::end:
      cross_offset = std::max(0.0f, inner_cross - cross);
      break;
    }

    const float x = origin_x + (horizontal ? main_cursor : options.padding.left + cross_offset);
    const float y = origin_y + (horizontal ? options.padding.top + cross_offset : main_cursor);
    const float width = horizontal ? value.main : cross;
    const float height = horizontal ? cross : value.main;
    const rect child_bounds{ x, y, width, height };

    detail::state_access::set_bounds(child.s, child_bounds);
    child.draw(ctx);

    main_cursor += value.main;
    ++placed;
  }
};

} // namespace impl

template <component_like... Ts>
[[nodiscard]] component<impl::flexbox<Ts...>> flexbox(state& s, flex_options options, Ts&... children) {
  static_assert((component_like<Ts> && ...), "flexbox children must satisfy component_like");
  return component<impl::flexbox<Ts...>>{ s, impl::flexbox<Ts...>{ options, std::tie(children...) } };
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
