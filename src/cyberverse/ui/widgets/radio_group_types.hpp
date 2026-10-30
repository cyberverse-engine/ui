/**
 * @file radio_group_types.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Radio group public configuration
 * @version 0.1
 * @date 2026-10-31
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets/stack_types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief How a radio group renders its options
 *
 */
enum class radio_group_type {
  radio,  ///< Native radio buttons
  button, ///< Buttons with a persistent selected style
};

/**
 * @brief Radio group configuration
 *
 */
struct radio_group_options {
  radio_group_type type = radio_group_type::radio;
  stack_options layout{};
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
