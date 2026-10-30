/**
 * @file text_input.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Text input component implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>
#include <string>

#include "cyberverse/ui/widgets2/context.hpp"
#include "cyberverse/ui/widgets2/state.hpp"
#include "cyberverse/ui/widgets2/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {
namespace impl {

/**
 * @brief Native single-line text input data
 *
 */
struct text_input {
  std::string* value = nullptr;
  std::string placeholder;
  bool password = false;
  bool read_only = false;
  std::optional<text_input_style> style_override;

  [[nodiscard]] size measure(const state& s, const theme& th, const constraints& lim) const noexcept;
  void draw(state& s, draw_context& ctx) const;
  template <typename F>
  void visit_states(F&& /*visitor*/) const noexcept {}
};

} // namespace impl
} // namespace widgets2
} // namespace ui
} // namespace cyberverse
