/**
 * @file rect.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Shared rectangle and layout size
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

namespace cyberverse {
namespace ui {

/**
 * @brief Rectangle in logical points
 *
 */
struct rect {
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
};

/**
 * @brief Width/height pair used by measurement and layout
 *
 */
struct layout_size {
  float width = 0.0f;
  float height = 0.0f;
};

} // namespace ui
} // namespace cyberverse
