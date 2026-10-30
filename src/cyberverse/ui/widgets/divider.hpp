/**
 * @file divider.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Lightweight horizontal or vertical line widget
 * @version 0.4
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>

#include "cyberverse/ui/widgets/widget.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Orientation of a divider line
 *
 */
enum class divider_orientation { horizontal, vertical };

/**
 * @brief Lightweight line that participates in flexbox layout
 *
 * A horizontal divider defaults to stretch across the parent content width and
 * a fixed thickness height; it belongs in a column flex container. A vertical
 * divider defaults to a fixed thickness width and stretches across the parent
 * content height; it belongs in a row flex container.
 *
 * Margins are regular flexbox margins, so a horizontal divider with left/right
 * margins occupies `parent_content_width - left_margin - right_margin`. The line
 * itself uses the theme divider border color (default ImGuiCol_Separator) unless
 * color() overrides it.
 *
 */
class divider final : public basic_widget<divider> {
public:
  explicit divider(divider_orientation orientation = divider_orientation::horizontal);

  /**
   * @brief Change orientation and reapply the orientation's default size policy
   *
   * This resets width/height to the orientation defaults. Call thickness() or an
   * explicit width/height after orientation() to override them.
   *
   */
  divider& orientation(divider_orientation value);

  /**
   * @brief Set line thickness in logical points and update the fixed axis
   *
   * A non-positive or non-finite value is treated as zero (the line is hidden).
   *
   */
  divider& thickness(float value);

  /**
   * @brief Override the line color; defaults to ImGuiCol_Separator
   *
   */
  divider& color(widgets::color value);

private:
  friend class detail::layout_engine;

  detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) override;
  void draw(detail::draw_context& context) override;
  void visit_children(const child_visitor& visitor) const override;

  void apply_orientation_defaults();

  divider_orientation orientation_ = divider_orientation::horizontal;
  float thickness_ = 1.0f;
  std::optional<widgets::color> color_;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
