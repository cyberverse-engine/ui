/**
 * @file text.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Text display widget
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>
#include <string>

#include "cyberverse/ui/widgets/widget.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Multi-line text with optional automatic wrapping
 *
 * Measurement follows Dear ImGui's current font and honors explicit newlines.
 * Drawing is clipped to the widget rectangle; content never changes the layout
 * rectangle once Yoga has assigned one.
 *
 */
class text final : public basic_widget<text> {
public:
  explicit text(std::string value);

  /**
   * @brief Enable automatic line wrapping at the widget width
   *
   */
  text& wrap(bool enabled = true);

  /**
   * @brief Override the resolved text color
   *
   */
  text& color(widgets::color value);

private:
  friend class detail::layout_engine;

  detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) override;
  void draw(detail::draw_context& context) override;
  void visit_children(const child_visitor& visitor) const override;

  std::string value_;
  bool wrap_ = false;
  std::optional<widgets::color> color_;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
