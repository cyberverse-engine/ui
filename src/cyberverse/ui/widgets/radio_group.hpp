/**
 * @file radio_group.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets radio group factory
 * @version 0.1
 * @date 2026-10-31
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <array>
#include <concepts>
#include <string>
#include <string_view>
#include <utility>

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/impl/radio_group.hpp"
#include "cyberverse/ui/widgets/radio_group_types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Create a mutually exclusive radio group backed by caller-owned selection
 *
 * Labels are copied. The selected value is an index in [0, labels.size()) and
 * is updated in place when an option is clicked. A change emits
 * event_type::value_changed on the group state. An out-of-range initial value
 * selects no option; clicking an option repairs it.
 *
 */
template <typename... Labels>
  requires(sizeof...(Labels) > 0 && (std::convertible_to<Labels, std::string_view> && ...))
[[nodiscard]] component<impl::radio_group<sizeof...(Labels)>>
radio_group(state& s, int& selected, radio_group_options options, Labels... labels) {
  std::array<std::string, sizeof...(Labels)> values{ std::string{ std::string_view{ labels } }... };
  return component<impl::radio_group<sizeof...(Labels)>>{
    s, impl::radio_group<sizeof...(Labels)>{ options, &selected, std::move(values) }
  };
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
