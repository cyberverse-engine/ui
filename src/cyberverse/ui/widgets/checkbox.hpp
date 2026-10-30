/**
 * @file checkbox.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets checkbox factory
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
#include "cyberverse/ui/widgets/impl/checkbox.hpp"
#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Create a native checkbox backed by caller-owned value
 *
 */
[[nodiscard]] inline component<impl::checkbox>
checkbox(state& s, std::string_view label, bool& value, std::optional<choice_style> style = std::nullopt) {
  return component<impl::checkbox>{ s, impl::checkbox{ std::string{ label }, &value, std::move(style) } };
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
