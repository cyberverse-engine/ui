/**
 * @file button.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native ImGui button adapter
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
 * @brief Standard button backed by Dear ImGui's native Button()
 *
 */
class button final : public basic_widget<button> {
public:
  explicit button(std::string label, std::function<void()> on_click = {});

private:
  friend class detail::layout_engine;

  detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) override;
  void draw(detail::draw_context& context) override;
  void visit_children(const child_visitor& visitor) const override;

  std::string label_;
  std::function<void()> on_click_;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
