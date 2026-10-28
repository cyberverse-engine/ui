/**
 * @file scroll_view.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Vertically scrollable flexbox container
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <type_traits>
#include <utility>

#include "cyberverse/ui/widgets/flexbox.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Container with a native Dear ImGui child scroll area
 *
 * The content is an internal column flexbox. A fit-content height grows to the
 * content; fixed/stretch heights keep the requested rectangle and let the native
 * child scroll vertically.
 *
 */
class scroll_view final : public basic_widget<scroll_view> {
public:
  scroll_view();

  scroll_view& direction(flex_direction value);
  scroll_view& gap(float value);
  scroll_view& padding(insets value);

  template <typename T>
  std::decay_t<T>& add(T&& child) {
    return content_.add(std::forward<T>(child));
  }

  [[nodiscard]] flexbox& content() noexcept { return content_; }

private:
  friend class detail::layout_engine;

  detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) override;
  void draw(detail::draw_context& context) override;
  void visit_children(const child_visitor& visitor) const override;

  flexbox content_;
  insets padding_{};
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
