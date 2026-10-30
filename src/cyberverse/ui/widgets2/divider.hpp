/**
 * @file divider.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets2 divider factory
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>
#include <utility>

#include "cyberverse/ui/widgets2/component.hpp"
#include "cyberverse/ui/widgets2/divider_types.hpp"
#include "cyberverse/ui/widgets2/impl/divider.hpp"
#include "cyberverse/ui/widgets2/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {

/**
 * @brief Create a horizontal or vertical divider
 *
 */
[[nodiscard]] inline component<impl::divider>
divider(state& s, divider_options options = {}, std::optional<divider_style> style = std::nullopt) {
  return component<impl::divider>{ s, impl::divider{ options, std::move(style) } };
}

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
