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

#include "cyberverse/ui/widgets2/component.hpp"
#include "cyberverse/ui/widgets2/stack_types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {

namespace impl {
template <typename... Ts>
struct stack;
}

/**
 * @brief Create a stack component from named child components
 *
 * The stack stores child references only; the children must outlive it.
 *
 */
template <component_like... Ts>
[[nodiscard]] component<impl::stack<Ts...>> stack(state& s, stack_options options, Ts&... children);

} // namespace widgets2
} // namespace ui
} // namespace cyberverse

#include "cyberverse/ui/widgets2/impl/stack.hpp"
