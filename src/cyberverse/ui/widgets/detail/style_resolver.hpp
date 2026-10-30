/**
 * @file style_resolver.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Theme/style resolution against the active ImGuiStyle
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <imgui.h>

#include <optional>

#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

enum class style_kind {
  panel,
  divider,
  button,
  checkbox,
  radio,
  text_input,
  scroll_view,
  collapse_header,
  collapse_body,
  text,
};

/**
 * @brief Concrete colors and metrics after theme/ImGuiStyle resolution
 *
 */
struct resolved_style {
  ImU32 background = 0;
  ImU32 background_hovered = 0;
  ImU32 background_active = 0;
  ImU32 text = 0;
  ImU32 text_disabled = 0;
  ImU32 border = 0;
  ImU32 focus_ring = 0;
  float border_width = 0.0f;
  float radius = 0.0f;
  float padding_x = 0.0f;
  float padding_y = 0.0f;
  bool has_background = true;
};

/**
 * @brief Resolve widget style: widget override > theme > ImGuiStyle
 *
 */
[[nodiscard]] resolved_style
resolve_style(style_kind kind, const theme& appearance, const std::optional<widget_style>& override);

/**
 * @brief Use a caller theme or a static empty theme during measurement
 *
 */
[[nodiscard]] const theme& appearance_or_default(const theme* appearance) noexcept;

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
