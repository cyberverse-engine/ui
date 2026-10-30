/**
 * @file text.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Text component implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>
#include <string_view>

#include "cyberverse/ui/widgets/context.hpp"
#include "cyberverse/ui/widgets/state.hpp"
#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

/**
 * @brief Text drawing data
 *
 */
struct text {
  std::string_view value;
  std::optional<text_style> style_override;

  [[nodiscard]] layout_size measure(const state& s, const theme& th, const constraints& lim) const noexcept;
  void draw(state& s, draw_context& ctx) const;
  template <typename F>
  void visit_states(F&& /*visitor*/) const noexcept {}
};

} // namespace impl
} // namespace widgets
} // namespace ui
} // namespace cyberverse
