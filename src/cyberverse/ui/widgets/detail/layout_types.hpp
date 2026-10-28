/**
 * @file layout_types.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Internal layout and draw types shared by widgets
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <imgui.h>

#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

enum class measure_mode { undefined, exactly, at_most };

/**
 * @brief Constraints passed to a widget's intrinsic measurement
 *
 */
struct constraints {
  float available_width = 0.0f;
  float available_height = 0.0f;
  measure_mode width_mode = measure_mode::undefined;
  measure_mode height_mode = measure_mode::undefined;
};

/**
 * @brief Intrinsic size returned by a widget measurement
 *
 */
struct measured_size {
  float width = 0.0f;
  float height = 0.0f;
};

/**
 * @brief Context for intrinsic measurement
 *
 */
struct measure_context {
  const theme* appearance = nullptr;
};

/**
 * @brief Context for one draw pass
 *
 */
struct draw_context {
  ImDrawList* draw_list = nullptr;
  const theme& appearance;
};

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
