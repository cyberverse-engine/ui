/**
 * @file widget.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Widget tree entry point
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/widget.hpp"

#include <algorithm>

#include <imgui.h>

#include "cyberverse/ui/widgets/detail/layout_engine.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

void render(widget& root, const theme& appearance) {
  if (ImGui::GetCurrentContext() == nullptr) {
    return;
  }

  const ImVec2 origin = ImGui::GetCursorScreenPos();
  const ImVec2 available = ImGui::GetContentRegionAvail();
  if (available.x <= 0.0f || available.y <= 0.0f) {
    return;
  }

  detail::render(root, origin, available, appearance);
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
