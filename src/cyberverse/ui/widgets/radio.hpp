/**
 * @file radio.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets radio factory
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/impl/radio.hpp"
#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Create a native radio button backed by caller-owned selection
 *
 */
[[nodiscard]] inline component<impl::radio>
radio(state& s, std::string_view label, int& selected, int value, std::optional<choice_style> style = std::nullopt) {
  return component<impl::radio>{ s, impl::radio{ std::string{ label }, &selected, value, std::move(style) } };
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
