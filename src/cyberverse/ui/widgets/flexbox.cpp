/**
 * @file flexbox.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Flexbox layout container implementation
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/flexbox.hpp"

#include "cyberverse/ui/widgets/panel.hpp"
#include "cyberverse/ui/widgets/stack.hpp"

#include "cyberverse/ui/widgets/detail/layout_types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

template <typename Derived>
detail::measured_size flex_container<Derived>::measure(detail::measure_context& /*context*/,
                                                       const detail::constraints& /*limits*/) {
  return {};
}

template <typename Derived>
void flex_container<Derived>::draw(detail::draw_context& /*context*/) {}

template <typename Derived>
void flex_container<Derived>::visit_children(const typename widget::child_visitor& visitor) const {
  for (const auto& child : children_) {
    if (child != nullptr) {
      visitor(*child);
    }
  }
}

template class flex_container<flexbox>;
template class flex_container<panel>;
template class flex_container<stack>;

} // namespace widgets
} // namespace ui
} // namespace cyberverse
