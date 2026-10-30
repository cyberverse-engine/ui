/**
 * @file z_order.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Window z-order protection through Dear ImGui internals
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/detail/z_order.hpp"

#include <imgui.h>
#include <imgui_internal.h>

namespace cyberverse {
namespace ui {
namespace detail {

void bring_window_to_front(const char* name) noexcept {
  if (name == nullptr || ImGui::GetCurrentContext() == nullptr) {
    return;
  }
  ImGuiWindow* window = ImGui::FindWindowByName(name);
  if (window == nullptr) {
    return;
  }
  ImGui::BringWindowToDisplayFront(window->RootWindow != nullptr ? window->RootWindow : window);
}

int display_order(const char* name_a, const char* name_b) noexcept {
  if (name_a == nullptr || name_b == nullptr || ImGui::GetCurrentContext() == nullptr) {
    return 0;
  }
  ImGuiWindow* window_a = ImGui::FindWindowByName(name_a);
  ImGuiWindow* window_b = ImGui::FindWindowByName(name_b);
  if (window_a == nullptr || window_b == nullptr) {
    return 0;
  }

  ImGuiContext& context = *GImGui;
  int index_a = -1;
  int index_b = -1;
  for (int index = 0; index < context.Windows.Size; ++index) {
    if (context.Windows[index] == window_a) {
      index_a = index;
    }
    if (context.Windows[index] == window_b) {
      index_b = index;
    }
  }

  if (index_a < 0 || index_b < 0 || index_a == index_b) {
    return 0;
  }
  return index_a < index_b ? -1 : 1;
}

} // namespace detail
} // namespace ui
} // namespace cyberverse
