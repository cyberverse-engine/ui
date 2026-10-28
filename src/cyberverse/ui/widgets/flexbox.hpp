/**
 * @file flexbox.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Flexbox layout container
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "cyberverse/ui/widgets/widget.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Shared implementation of flex container widgets
 *
 * Concrete containers derive from flex_container<Derived> so that every fluent
 * sizing/style setter returns the concrete type (for example panel&).
 *
 */
template <typename Derived>
class flex_container : public basic_widget<Derived> {
public:
  explicit flex_container(flex_direction direction) : direction_{ direction } {}

  Derived& direction(flex_direction value) {
    direction_ = value;
    return static_cast<Derived&>(*this);
  }

  Derived& gap(float value) {
    gap_ = value;
    return static_cast<Derived&>(*this);
  }

  Derived& padding(insets value) {
    padding_ = value;
    return static_cast<Derived&>(*this);
  }

  Derived& justify(justify_content value) {
    justify_ = value;
    return static_cast<Derived&>(*this);
  }

  Derived& align(align_items value) {
    align_ = value;
    return static_cast<Derived&>(*this);
  }

  /**
   * @brief Move a widget into this container and return a stable reference
   *
   * The reference is valid until the containing tree is destroyed (normally at
   * the end of widgets::render). Add consumes its argument; widgets are
   * non-copyable, so fluent temporaries are moved into the container.
   *
   */
  template <typename T>
  std::decay_t<T>& add(T&& child) {
    using value_type = std::decay_t<T>;
    static_assert(std::is_base_of_v<widget, value_type>, "container children must derive from widgets::widget");
    auto owned = std::make_unique<value_type>(std::move(child));
    value_type& reference = *owned;
    children_.push_back(std::move(owned));
    return reference;
  }

private:
  friend class detail::layout_engine;

  detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) override;
  void draw(detail::draw_context& context) override;
  void visit_children(const typename widget::child_visitor& visitor) const override;
  [[nodiscard]] bool is_measure_leaf() const noexcept override { return false; }
  [[nodiscard]] flex_direction flow_direction() const noexcept override { return direction_; }
  [[nodiscard]] insets flow_padding() const noexcept override { return padding_; }
  [[nodiscard]] float flow_gap() const noexcept override { return gap_; }
  [[nodiscard]] justify_content flow_justify() const noexcept override { return justify_; }
  [[nodiscard]] align_items flow_align() const noexcept override { return align_; }

  std::vector<std::unique_ptr<widget>> children_;
  flex_direction direction_ = flex_direction::column;
  float gap_ = 0.0f;
  insets padding_{};
  justify_content justify_ = justify_content::start;
  align_items align_ = align_items::start;
};

/**
 * @brief Nestable flexbox container
 *
 * v1 does not support flex-wrap; row/column and their reverse directions are
 * supported.
 *
 */
class flexbox final : public flex_container<flexbox> {
public:
  explicit flexbox(flex_direction direction = flex_direction::column) : flex_container<flexbox>(direction) {}
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
