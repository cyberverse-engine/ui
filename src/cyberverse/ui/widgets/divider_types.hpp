/**
 * @file divider_types.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Divider public configuration
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Divider orientation
 *
 */
enum class divider_orientation { horizontal, vertical };

/**
 * @brief Divider configuration
 *
 */
struct divider_options {
  divider_orientation orientation = divider_orientation::horizontal;
  float thickness = 1.0f;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
