/**
 * @file box.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets box factory
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>
#include <utility>

#include "cyberverse/ui/widgets/box_types.hpp"
#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/impl/box.hpp"
#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Create a styled single-child box
 *
 */
template <component_like Child>
[[nodiscard]] component<impl::box<Child>>
box(state& s, box_options options, Child& child, std::optional<box_style> style = std::nullopt) {
  return component<impl::box<Child>>{ s, impl::box<Child>{ options, std::move(style), child } };
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
