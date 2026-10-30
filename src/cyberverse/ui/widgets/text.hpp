/**
 * @file text.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets text factory
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>
#include <string_view>
#include <utility>

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/impl/text.hpp"
#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Create a text component
 *
 * @param s Persisted component state
 * @param value Caller-owned text
 * @param style Optional full style override; empty uses theme.text
 */
[[nodiscard]] inline component<impl::text>
text(state& s, std::string_view value, std::optional<text_style> style = std::nullopt) {
  return component<impl::text>{ s, impl::text{ value, std::move(style) } };
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
