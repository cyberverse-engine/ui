/**
 * @file stack_layout.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Shared stack layout calculations
 * @version 0.1
 * @date 2026-10-31
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>

#include "cyberverse/ui/widgets/stack_types.hpp"
#include "cyberverse/ui/widgets/types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

/**
 * @brief Measure a stack-sized group of items
 *
 */
template <std::size_t N>
[[nodiscard]] layout_size measure_stack(const stack_options& options,
                                        const std::array<layout_size, N>& sizes,
                                        const std::array<bool, N>& active) noexcept {
  const bool horizontal = options.direction == stack_direction::horizontal;
  float main = 0.0f;
  float cross = 0.0f;
  std::size_t active_count = 0;

  for (std::size_t index = 0; index < N; ++index) {
    if (!active[index]) {
      continue;
    }
    if (active_count > 0) {
      main += std::max(0.0f, options.gap);
    }
    main += horizontal ? sizes[index].width : sizes[index].height;
    cross = std::max(cross, horizontal ? sizes[index].height : sizes[index].width);
    ++active_count;
  }

  main += horizontal ? options.padding.left + options.padding.right : options.padding.top + options.padding.bottom;
  cross += horizontal ? options.padding.top + options.padding.bottom : options.padding.left + options.padding.right;

  return horizontal ? layout_size{ std::max(0.0f, main), std::max(0.0f, cross) }
                    : layout_size{ std::max(0.0f, cross), std::max(0.0f, main) };
}

/**
 * @brief Place stack items inside bounds using top-left cross-axis alignment
 *
 */
template <std::size_t N>
[[nodiscard]] std::array<rect, N> place_stack(const stack_options& options,
                                              const std::array<layout_size, N>& sizes,
                                              const std::array<bool, N>& active,
                                              const rect& bounds) noexcept {
  std::array<rect, N> result{};
  const bool horizontal = options.direction == stack_direction::horizontal;
  const float gap = std::max(0.0f, options.gap);
  float cursor = 0.0f;
  bool first = true;

  for (std::size_t index = 0; index < N; ++index) {
    if (!active[index]) {
      continue;
    }
    if (!first) {
      cursor += gap;
    }
    first = false;

    float x = bounds.x + options.padding.left;
    float y = bounds.y + options.padding.top;
    if (horizontal) {
      x += cursor;
      cursor += sizes[index].width;
    } else {
      y += cursor;
      cursor += sizes[index].height;
    }
    result[index] = rect{ x, y, sizes[index].width, sizes[index].height };
  }
  return result;
}

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
