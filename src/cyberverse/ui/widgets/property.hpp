/**
 * @file property.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Animatable component properties
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstddef>

#include "cyberverse/animation/easing.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief All animatable property values are floats
 *
 */
enum class property_type {
  width,
  height,
  opacity,
  font_size,
  flex_grow,
  flex_shrink,
  flex_basis,
  scroll_x,
  scroll_y,
  property_size,
};

inline constexpr std::size_t property_count = static_cast<std::size_t>(property_type::property_size);

/**
 * @brief Parameters for an animated property change
 *
 */
struct animation_params {
  float duration = 0.0f;
  float delay = 0.0f;
  animation::easing_type easing = animation::easing_type::ease_out_cubic;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
