/**
 * @file checkbox.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native ImGui checkbox adapter
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
 * @brief Standard checkbox backed by Dear ImGui's native Checkbox()
 *
 * The checked value is application-owned and outlives the per-frame widget tree.
 *
 */
class checkbox final : public basic_widget<checkbox> {
public:
  checkbox(std::string label, bool& value);

private:
  friend class detail::layout_engine;

  detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) override;
  void draw(detail::draw_context& context) override;
  void visit_children(const child_visitor& visitor) const override;

  std::string label_;
  bool* value_ = nullptr;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
