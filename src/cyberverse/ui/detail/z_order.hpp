/**
 * @file z_order.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Layer z-order protection for Dear ImGui windows
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

namespace cyberverse {
namespace ui {
namespace detail {

/**
 * @brief Raise one managed panel window above lower windows
 *
 * The Dear ImGui window name must be null-terminated. A missing window is
 * ignored. This is the only UI component that relies on imgui_internal.h.
 *
 * @param name Managed window name
 */
void bring_window_to_front(const char* name) noexcept;

/**
 * @brief Compare the display order of two managed windows
 *
 * @return 1 when a is in front of b, -1 when it is behind, 0 when either window
 *         is unknown or both are the same
 */
[[nodiscard]] int display_order(const char* name_a, const char* name_b) noexcept;

} // namespace detail
} // namespace ui
} // namespace cyberverse
