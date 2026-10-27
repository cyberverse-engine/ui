/**
 * @file input_bridge.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief platform raw input to Dear ImGui event bridge
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <bitset>
#include <cstddef>

#include <imgui.h>

#include "cyberverse/platform/raw_input.hpp"

namespace cyberverse {
namespace ui {
namespace detail {

/**
 * @brief Feeds platform::raw_input_state into Dear ImGui
 *
 * The bridge is single-threaded and stateful: it only emits key and mouse
 * button edges when the platform state changes, and it keeps the modifier keys
 * in sync so focus loss cannot leave a key stuck.
 *
 */
class input_bridge {
public:
  /**
   * @brief Number of mouse buttons forwarded to Dear ImGui
   *
   */
  static constexpr int mouse_button_count = 5;

  /**
   * @brief Forward one input frame to Dear ImGui
   *
   * @param io Dear ImGui IO of the active context
   * @param input Platform input state for this frame
   */
  void update(ImGuiIO& io, const platform::raw_input_state& input);

  /**
   * @brief Forget all forwarded state
   *
   * The next update emits the full current state as edges.
   *
   */
  void reset() noexcept;

private:
  std::bitset<static_cast<std::size_t>(platform::key_id::max_value)> keys_down_;
  std::bitset<mouse_button_count> mouse_down_;
  bool control_down_ = false;
  bool shift_down_ = false;
  bool alt_down_ = false;
  bool super_down_ = false;
};

} // namespace detail
} // namespace ui
} // namespace cyberverse
