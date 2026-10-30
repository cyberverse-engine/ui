/**
 * @file animator.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Global animation policy for widgets2 state objects
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <cmath>

namespace cyberverse {
namespace ui {
namespace widgets2 {

/**
 * @brief Global animation switches; owns no per-property animation data
 *
 */
class animator {
public:
  [[nodiscard]] bool paused() const noexcept { return paused_; }

  void paused(bool value) noexcept { paused_ = value; }

  [[nodiscard]] float speed() const noexcept { return speed_; }

  void speed(float value) noexcept {
    speed_ = (std::isfinite(value) && value >= 0.0f) ? std::min(value, 100.0f) : 1.0f;
  }

private:
  bool paused_ = false;
  float speed_ = 1.0f;
};

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
