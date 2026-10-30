/**
 * @file divider.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets divider factory
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>
#include <utility>

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/divider_types.hpp"
#include "cyberverse/ui/widgets/impl/divider.hpp"
#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Create a horizontal or vertical divider
 *
 */
[[nodiscard]] inline component<impl::divider>
divider(state& s, divider_options options = {}, std::optional<divider_style> style = std::nullopt) {
  return component<impl::divider>{ s, impl::divider{ options, std::move(style) } };
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
