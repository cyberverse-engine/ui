/**
 * @file theme.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets2 appearance configuration
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

namespace cyberverse {
namespace ui {
namespace widgets2 {

/**
 * @brief Straight-alpha color
 *
 */
struct color {
  float r = 0.0f;
  float g = 0.0f;
  float b = 0.0f;
  float a = 1.0f;
};

/**
 * @brief Text appearance
 *
 */
struct text_style {
  color text{ 0.92f, 0.94f, 0.98f, 1.0f };
};

/**
 * @brief Button appearance
 *
 */
struct button_style {
  color background{ 0.18f, 0.25f, 0.34f, 1.0f };
  color background_hovered{ 0.24f, 0.34f, 0.45f, 1.0f };
  color background_active{ 0.30f, 0.43f, 0.57f, 1.0f };
  color text{ 0.92f, 0.94f, 0.98f, 1.0f };
  float padding_x = 8.0f;
  float padding_y = 4.0f;
  float rounding = 4.0f;
  float border_width = 1.0f;
};

/**
 * @brief Global appearance passed to widgets2::run
 *
 */
struct theme {
  float font_size = 0.0f; ///< Base font size; 0 uses Dear ImGui's current size
  text_style text;
  button_style button;
};

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
