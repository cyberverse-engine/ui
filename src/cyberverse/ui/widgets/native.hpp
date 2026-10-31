/**
 * @file native.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native ImGui component factory
 * @version 0.1
 * @date 2026-10-31
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/impl/native.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Create a component that measures and draws raw ImGui content
 *
 * @param s Caller-owned state
 * @param measure Callable(theme, constraints) -> layout_size
 * @param draw Callable(widget_context&, bounds)
 */
template <typename Measure, typename Draw>
[[nodiscard]] component<impl::native<Measure, Draw>> native(state& s, Measure measure, Draw draw) {
  return component<impl::native<Measure, Draw>>{ s,
                                                 impl::native<Measure, Draw>{ std::move(measure), std::move(draw) } };
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
