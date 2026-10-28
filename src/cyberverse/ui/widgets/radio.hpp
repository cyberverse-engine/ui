/**
 * @file radio.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native ImGui radio button adapter
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <string>

#include "cyberverse/ui/widgets/widget.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Standard radio option backed by Dear ImGui's native RadioButton()
 *
 * Several radio widgets share one application-owned integer selection value. The
 * widget is selected when selected == value.
 *
 */
class radio final : public basic_widget<radio> {
public:
  radio(std::string label, int& selected, int value);

private:
  friend class detail::layout_engine;

  detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) override;
  void draw(detail::draw_context& context) override;
  void visit_children(const child_visitor& visitor) const override;

  std::string label_;
  int* selected_ = nullptr;
  int value_ = 0;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
