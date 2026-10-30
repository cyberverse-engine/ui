/**
 * @file stack.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Stack implementation
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

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/context.hpp"
#include "cyberverse/ui/widgets/detail/stack_layout.hpp"
#include "cyberverse/ui/widgets/detail/state_access.hpp"
#include "cyberverse/ui/widgets/stack_types.hpp"
#include "cyberverse/ui/widgets/theme.hpp"
#include "cyberverse/ui/widgets/types.hpp"
#include "cyberverse/ui/widgets/visibility.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
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

  [[nodiscard]] layout_size measure(const state& s, const theme& th, const constraints& lim) const noexcept {
    constexpr std::size_t count = sizeof...(Ts);
    std::array<layout_size, count> sizes{};
    std::array<bool, count> active{};

    std::size_t index = 0;
    std::apply(
        [&](auto&... child) {
          ((active[index] = child.visibility_value() != visibility::collapsed && !child.s.has_zero_dimension(),
            sizes[index] = active[index] ? child.measure(th, lim) : layout_size{},
            ++index),
           ...);
        },
        children);

    layout_size result = detail::measure_stack(options, sizes, active);
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

    constexpr std::size_t count = sizeof...(Ts);
    std::array<layout_size, count> sizes{};
    std::array<bool, count> active{};

    std::size_t index = 0;
    std::apply(
        [&](auto&... child) {
          ((active[index] = child.visibility_value() != visibility::collapsed && !child.s.has_zero_dimension(),
            sizes[index] = active[index] ? child.measure(ctx.appearance, constraints{ bounds.width, bounds.height })
                                         : layout_size{},
            ++index),
           ...);
        },
        children);

    const std::array<rect, count> placements = detail::place_stack(options, sizes, active, bounds);
    index = 0;
    std::apply(
        [&](auto&... child) {
          ((detail::state_access::set_bounds(child.s, placements[index]),
            active[index] ? child.draw(ctx) : void(),
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
};

} // namespace impl

/**
 * @brief Create a stack component from named child components
 *
 */
template <component_like... Ts>
[[nodiscard]] component<impl::stack<Ts...>> stack(state& s, stack_options options, Ts&... children) {
  static_assert((component_like<Ts> && ...), "stack children must satisfy component_like");
  return component<impl::stack<Ts...>>{ s, impl::stack<Ts...>{ options, std::tie(children...) } };
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
