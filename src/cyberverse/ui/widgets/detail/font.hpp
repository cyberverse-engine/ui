/**
 * @file font.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Internal font and color helpers for widgets
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <cmath>

#include <imgui.h>

#include "cyberverse/ui/widgets/state.hpp"
#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

/**
 * @brief Resolve the base font size for a component
 *
 * Returns 0 when the current Dear ImGui font size should be used.
 *
 */
[[nodiscard]] inline float resolve_font_size(const state& s, const theme& th) noexcept {
  if (s.has(property_type::font_size)) {
    const float value = s.value(property_type::font_size);
    if (std::isfinite(value) && value > 0.0f) {
      return value;
    }
  }
  if (std::isfinite(th.font_size) && th.font_size > 0.0f) {
    return th.font_size;
  }
  return 0.0f;
}

/**
 * @brief Balances ImGui::PushFont for the duration of a component operation
 *
 */
class font_scope {
public:
  explicit font_scope(float size) noexcept {
    if (std::isfinite(size) && size > 0.0f) {
      ImGui::PushFont(nullptr, size);
      pushed_ = true;
    }
  }

  ~font_scope() {
    if (pushed_) {
      ImGui::PopFont();
    }
  }

  font_scope(const font_scope&) = delete;
  font_scope& operator=(const font_scope&) = delete;

private:
  bool pushed_ = false;
};

[[nodiscard]] inline ImU32 to_u32(const color& value, float opacity = 1.0f) noexcept {
  const float alpha = std::clamp(value.a * opacity, 0.0f, 1.0f);
  return ImGui::ColorConvertFloat4ToU32(ImVec4{ value.r, value.g, value.b, alpha });
}

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
