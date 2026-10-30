/**
 * @file text_input.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets2 text input factory
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

#include "cyberverse/ui/widgets2/component.hpp"
#include "cyberverse/ui/widgets2/impl/text_input.hpp"
#include "cyberverse/ui/widgets2/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {

/**
 * @brief Options for a single-line text input
 *
 */
struct text_input_options {
  std::string_view placeholder;
  bool password = false;
  bool read_only = false;
};

/**
 * @brief Create a native single-line text input backed by caller-owned text
 *
 */
[[nodiscard]] inline component<impl::text_input> text_input(state& s,
                                                            std::string& value,
                                                            text_input_options options = {},
                                                            std::optional<text_input_style> style = std::nullopt) {
  return component<impl::text_input>{
    s,
    impl::text_input{
        &value, std::string{ options.placeholder }, options.password, options.read_only, std::move(style) }
  };
}

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
