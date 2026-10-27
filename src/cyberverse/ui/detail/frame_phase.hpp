/**
 * @file frame_phase.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Thread-local UI frame marker for structural mutation checks
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <stdexcept>
#include <string>

namespace cyberverse {
namespace ui {
namespace detail {

inline thread_local bool g_frame_in_progress = false;

/**
 * @brief Whether the calling thread is inside an active UI frame
 *
 */
[[nodiscard]] inline bool frame_in_progress() noexcept { return g_frame_in_progress; }

/**
 * @brief Set the calling thread's UI frame marker
 *
 */
inline void set_frame_in_progress(bool value) noexcept { g_frame_in_progress = value; }

/**
 * @brief Reject structural mutations during an active frame
 *
 * @throws std::logic_error When a frame is active
 */
inline void require_between_frames(const char* operation) {
  if (g_frame_in_progress) {
    throw std::logic_error(std::string{ "cyberverse::ui: cannot " } + operation + " during an active UI frame");
  }
}

} // namespace detail
} // namespace ui
} // namespace cyberverse
