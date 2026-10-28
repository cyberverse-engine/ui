/**
 * @file theme.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Widget appearance types
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <optional>

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Straight-alpha color in the range [0, 1]
 *
 */
struct color {
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 1.0f;
};

/**
 * @brief Solid border description
 *
 */
struct border_style {
  color color{};       ///< Border color
  float width = 1.0f;  ///< Border width in logical points
  float radius = 4.0f; ///< Corner radius in logical points
};

/**
 * @brief Optional appearance overrides for one widget kind
 *
 * Every field is optional. Missing fields are inherited from the active theme,
 * then from the current Dear ImGui style. A widget-level style override uses the
 * same type and takes precedence over the theme.
 *
 */
struct widget_style {
  std::optional<color> background;         ///< Idle background
  std::optional<color> background_hovered; ///< Hovered background
  std::optional<color> background_active;  ///< Pressed background
  std::optional<color> text;               ///< Primary text color
  std::optional<color> text_disabled;      ///< Disabled text color
  std::optional<border_style> border;      ///< Solid border and corner radius
  std::optional<color> focus_ring;         ///< Keyboard focus ring color
  std::optional<float> padding_x;          ///< Horizontal content padding
  std::optional<float> padding_y;          ///< Vertical content padding
};

/**
 * @brief Optional appearance overrides passed to widgets::render()
 *
 */
struct theme {
  widget_style panel;
  widget_style button;
  widget_style checkbox;
  widget_style radio;
  widget_style text_input;
  widget_style scroll_view;
  widget_style collapse_header;
  widget_style collapse_body;
  widget_style text;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
