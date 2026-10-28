/**
 * @file collapse_group.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Collapsible titled container
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <string>
#include <type_traits>
#include <utility>

#include "cyberverse/ui/widgets/flexbox.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Titled container whose body can be collapsed
 *
 * The open state is application-owned. When closed the body is neither measured
 * nor drawn. The header is a thin interactive row; the body is an internal
 * column flexbox.
 *
 */
class collapse_group final : public basic_widget<collapse_group> {
public:
  collapse_group(std::string title, bool& open);

  collapse_group& gap(float value);
  collapse_group& padding(insets value);

  template <typename T>
  std::decay_t<T>& add(T&& child) {
    return body_.add(std::forward<T>(child));
  }

  [[nodiscard]] flexbox& body() noexcept { return body_; }

private:
  friend class detail::layout_engine;

  detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) override;
  void draw(detail::draw_context& context) override;
  void visit_children(const child_visitor& visitor) const override;

  std::string title_;
  bool* open_ = nullptr;
  flexbox body_;
  float gap_ = 0.0f;
  insets padding_{};
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
