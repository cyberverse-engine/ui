/**
 * @file panel.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Styled flexbox container
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets/flexbox.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Flexbox container with a background, border, and corner radius
 *
 * The default visual style comes from the active theme/ImGuiStyle; use
 * style() or the theme passed to render() to override it.
 *
 */
class panel final : public flex_container<panel> {
public:
  panel() : flex_container<panel>(flex_direction::column) {}

private:
  friend class detail::layout_engine;

  void draw(detail::draw_context& context) override;
  [[nodiscard]] bool clips_children() const noexcept override { return true; }
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
