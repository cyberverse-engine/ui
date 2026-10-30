/**
 * @file scroll_view.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Compile-time dual-axis scroll view
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>

#include "cyberverse/ui/widgets2/component.hpp"
#include "cyberverse/ui/widgets2/scroll_view_types.hpp"
#include "cyberverse/ui/widgets2/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {

namespace impl {
template <typename... Ts>
struct scroll_view;
}

/**
 * @brief Create a dual-axis scroll view from named child components
 *
 * Scroll offsets live in state.scroll_x and state.scroll_y.
 *
 */
template <component_like... Ts>
[[nodiscard]] component<impl::scroll_view<Ts...>>
scroll_view(state& s, scroll_options options, std::optional<scroll_view_style> style, Ts&... children);

template <component_like... Ts>
[[nodiscard]] component<impl::scroll_view<Ts...>> scroll_view(state& s, scroll_options options, Ts&... children);

} // namespace widgets2
} // namespace ui
} // namespace cyberverse

#include "cyberverse/ui/widgets2/impl/scroll_view.hpp"
