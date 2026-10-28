/**
 * @file native.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Escape hatch for third-party Dear ImGui content
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <functional>
#include <string>

#include "cyberverse/ui/widgets/widget.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Widget that hands an explicit rectangle to arbitrary ImGui code
 *
 * This is the integration point for third-party domain libraries such as ImPlot,
 * ImNodes, or ImSpinner. The callback runs inside the current Dear ImGui window
 * with the cursor positioned at the rectangle; it must not create a top-level
 * window. The callback should draw inside the given rect. Use the optional
 * measure callback to provide a fit-content size; otherwise the widget fills the
 * available constraints.
 *
 */
class native final : public basic_widget<native> {
public:
  using draw_fn = std::function<void(const rect&)>;
  using measure_fn = std::function<content_size()>;

  native(std::string id, draw_fn draw, measure_fn measure = {});

private:
  friend class detail::layout_engine;

  detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) override;
  void draw(detail::draw_context& context) override;
  void visit_children(const child_visitor& visitor) const override;

  draw_fn draw_;
  measure_fn measure_;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
