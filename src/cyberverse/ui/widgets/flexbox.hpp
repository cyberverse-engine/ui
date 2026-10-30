/**
 * @file flexbox.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Compile-time flexbox container
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/flex_types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

namespace impl {
template <typename... Ts>
struct flexbox;
}

/**
 * @brief Create a no-wrap flexbox from named child components
 *
 * Flex item values are read from each child state:
 * flex_grow, flex_shrink, flex_basis, width, and height. The flexbox writes
 * child bounds only; it never writes the child sizing properties.
 *
 */
template <component_like... Ts>
[[nodiscard]] component<impl::flexbox<Ts...>> flexbox(state& s, flex_options options, Ts&... children);

} // namespace widgets
} // namespace ui
} // namespace cyberverse

#include "cyberverse/ui/widgets/impl/flexbox.hpp"
