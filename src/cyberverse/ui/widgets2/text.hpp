/**
 * @file text.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets2 text factory
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

#include "cyberverse/ui/widgets2/component.hpp"
#include "cyberverse/ui/widgets2/impl/text.hpp"
#include "cyberverse/ui/widgets2/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {

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

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
