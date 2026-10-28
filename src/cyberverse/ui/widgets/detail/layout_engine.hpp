/**
 * @file layout_engine.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Yoga-backed widget layout and draw scheduling
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/layout_types.hpp"
#include "cyberverse/ui/widgets/widget.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

/**
 * @brief Lay out and draw one widget tree
 *
 * The root is placed at @p origin and calculated against @p available. All
 * widget bounds are written in screen coordinates. The function requires an
 * active Dear ImGui frame.
 *
 */
void render(widget& root, ImVec2 origin, ImVec2 available, const theme& appearance);

/**
 * @brief Measure a widget subtree without drawing it
 *
 * Used by containers whose content has an independent layout pass
 * (scroll_view/collapse_group).
 *
 */
[[nodiscard]] measured_size measure(widget& root, const constraints& limits, const theme& appearance);

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
