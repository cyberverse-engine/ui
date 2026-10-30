/**
 * @file stack.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Compile-time stack container
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
#include <tuple>
#include <utility>

#include "cyberverse/ui/widgets2/component.hpp"
#include "cyberverse/ui/widgets2/context.hpp"
#include "cyberverse/ui/widgets2/detail/state_access.hpp"
#include "cyberverse/ui/widgets2/rect.hpp"
#include "cyberverse/ui/widgets2/theme.hpp"
#include "cyberverse/ui/widgets2/visibility.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {

/**
 * @brief Four-sided logical inset
 *
 */
struct insets {
  float left = 0.0f;
  float top = 0.0f;
  float right = 0.0f;
  float bottom = 0.0f;
};

/**
 * @brief Stack layout direction
 *
 */
enum class stack_direction { horizontal, vertical };

/**
 * @brief Stack layout configuration
 *
 */
struct stack_options {
  stack_direction direction = stack_direction::vertical;
  float gap = 0.0f;
  insets padding{};
};

namespace impl {

/**
 * @brief Stack implementation holding references to components
 *
 * The tuple contains references only; child components are caller-owned and
 * never copied or allocated by the stack.
 *
 */
template <typename... Ts>
struct stack {
  stack_options options;
  std::tuple<Ts&...> children;

  stack(stack_options options_, std::tuple<Ts&...> children_) : options(options_), children(children_) {}

  [[nodiscard]] size measure(const state& s, const theme& th, const constraints& lim) const noexcept {
    constexpr std::size_t count = sizeof...(Ts);
    std::array<size, count> sizes{};
    std::array<bool, count> active{};

    std::size_t index = 0;
    std::apply(
        [&](auto&... child) {
          ((active[index] = child.visibility_value() != visibility::collapsed,
            sizes[index] = active[index] ? child.measure(th, lim) : size{},
            ++index),
           ...);
        },
        children);

    const bool horizontal = options.direction == stack_direction::horizontal;
    float main = 0.0f;
    float cross = 0.0f;
    std::size_t active_count = 0;

    for (std::size_t i = 0; i < count; ++i) {
      if (!active[i]) {
        continue;
      }
      if (active_count > 0) {
        main += std::max(0.0f, options.gap);
      }
      main += horizontal ? sizes[i].width : sizes[i].height;
      cross = std::max(cross, horizontal ? sizes[i].height : sizes[i].width);
      ++active_count;
    }

    main += horizontal ? options.padding.left + options.padding.right : options.padding.top + options.padding.bottom;
    cross += horizontal ? options.padding.top + options.padding.bottom : options.padding.left + options.padding.right;

    size result = horizontal ? size{ main, cross } : size{ cross, main };
    if (s.has(property_type::width)) {
      result.width = std::max(0.0f, s.value(property_type::width));
    }
    if (s.has(property_type::height)) {
      result.height = std::max(0.0f, s.value(property_type::height));
    }
    return result;
  }

  void draw(state& s, draw_context& ctx) const {
    const rect& bounds = s.bounds();
    if (bounds.width <= 0.0f || bounds.height <= 0.0f) {
      return;
    }

    constexpr std::size_t count = sizeof...(Ts);
    std::array<size, count> sizes{};
    std::array<bool, count> active{};

    std::size_t index = 0;
    std::apply(
        [&](auto&... child) {
          ((active[index] = child.visibility_value() != visibility::collapsed,
            sizes[index] =
                active[index] ? child.measure(ctx.appearance, constraints{ bounds.width, bounds.height }) : size{},
            ++index),
           ...);
        },
        children);

    const bool horizontal = options.direction == stack_direction::horizontal;
    const float gap = std::max(0.0f, options.gap);
    float cursor_x = bounds.x + options.padding.left;
    float cursor_y = bounds.y + options.padding.top;
    bool first = true;

    index = 0;
    std::apply(
        [&](auto&... child) {
          ((draw_one(child, sizes[index], active[index], horizontal, gap, first, cursor_x, cursor_y, ctx), ++index),
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
  static void draw_one(Child& child,
                       const size& child_size,
                       bool is_active,
                       bool horizontal,
                       float gap,
                       bool& first,
                       float& cursor_x,
                       float& cursor_y,
                       draw_context& ctx) {
    if (!is_active) {
      detail::state_access::set_bounds(child.s, rect{});
      return;
    }

    if (!first) {
      if (horizontal) {
        cursor_x += gap;
      } else {
        cursor_y += gap;
      }
    }
    first = false;

    const rect child_bounds{ cursor_x, cursor_y, child_size.width, child_size.height };
    detail::state_access::set_bounds(child.s, child_bounds);
    child.draw(ctx);

    if (horizontal) {
      cursor_x += child_size.width;
    } else {
      cursor_y += child_size.height;
    }
  }
};

} // namespace impl

/**
 * @brief Create a stack component from named child components
 *
 * The stack stores child references only; the children must outlive it.
 *
 */
template <component_like... Ts>
[[nodiscard]] component<impl::stack<Ts...>> stack(state& s, stack_options options, Ts&... children) {
  static_assert((component_like<Ts> && ...), "stack children must satisfy component_like");
  return component<impl::stack<Ts...>>{ s, impl::stack<Ts...>{ options, std::tie(children...) } };
}

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
