/**
 * @file button.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets2 button factory
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
#include "cyberverse/ui/widgets2/impl/button.hpp"
#include "cyberverse/ui/widgets2/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {

/**
 * @brief Create a button component
 *
 * Click handling lives in state::on_event(event_type::click). The optional
 * style replaces theme.button completely for this component.
 *
 */
[[nodiscard]] inline component<impl::button>
button(state& s, std::string_view label, std::optional<button_style> style = std::nullopt) {
  return component<impl::button>{ s, impl::button{ label, std::move(style) } };
}

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
