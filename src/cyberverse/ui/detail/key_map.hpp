/**
 * @file key_map.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief platform::key_id to ImGuiKey mapping table
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstddef>

#include <imgui.h>

#include "cyberverse/platform/raw_input.hpp"

namespace cyberverse {
namespace ui {
namespace detail {

/**
 * @brief One platform-to-ImGui keyboard key mapping
 *
 */
struct key_binding {
  platform::key_id key; ///< Platform key id
  ImGuiKey imgui;       ///< Dear ImGui key
};

/**
 * @brief Keyboard bindings sent to Dear ImGui every time they change
 *
 * Mouse buttons are handled separately by input_bridge. Keys missing from this
 * table are ignored until the full M4 mapping lands.
 *
 */
[[nodiscard]] const key_binding* key_bindings() noexcept;

/**
 * @brief Number of entries returned by key_bindings()
 *
 */
[[nodiscard]] std::size_t key_binding_count() noexcept;

} // namespace detail
} // namespace ui
} // namespace cyberverse
