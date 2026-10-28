/**
 * @file text_input.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native single-line ImGui text input
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <functional>
#include <string>
#include <string_view>

#include "cyberverse/ui/widgets/widget.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Single-line text input backed by Dear ImGui's native InputText()
 *
 * The value is application-owned and survives widget-tree rebuilds. The widget
 * relies on the ui input bridge for committed UTF-8 text, so IME commit input
 * works through the same path as ordinary typing.
 *
 */
class text_input final : public basic_widget<text_input> {
public:
  text_input(std::string id, std::string& value);

  text_input& placeholder(std::string_view value);
  text_input& password(bool enabled = true);
  text_input& read_only(bool enabled = true);
  text_input& on_change(std::function<void(std::string_view)> callback);

private:
  friend class detail::layout_engine;

  detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) override;
  void draw(detail::draw_context& context) override;
  void visit_children(const child_visitor& visitor) const override;

  std::string* value_ = nullptr;
  std::string placeholder_;
  bool password_ = false;
  bool read_only_ = false;
  std::function<void(std::string_view)> on_change_;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
