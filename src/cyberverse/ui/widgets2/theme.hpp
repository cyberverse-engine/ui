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
 * @brief Checkbox and radio appearance
 *
 */
struct choice_style {
  color frame{ 0.12f, 0.16f, 0.22f, 1.0f };
  color frame_hovered{ 0.18f, 0.24f, 0.32f, 1.0f };
  color frame_active{ 0.22f, 0.30f, 0.40f, 1.0f };
  color border{ 0.32f, 0.40f, 0.50f, 1.0f };
  color text{ 0.92f, 0.94f, 0.98f, 1.0f };
  color check{ 0.42f, 0.72f, 0.94f, 1.0f };
  float padding_x = 2.0f;
  float padding_y = 2.0f;
  float rounding = 3.0f;
  float border_width = 1.0f;
};

/**
 * @brief Single-line text input appearance
 *
 */
struct text_input_style {
  color background{ 0.10f, 0.13f, 0.18f, 1.0f };
  color background_hovered{ 0.15f, 0.20f, 0.27f, 1.0f };
  color background_active{ 0.19f, 0.25f, 0.34f, 1.0f };
  color border{ 0.32f, 0.40f, 0.50f, 1.0f };
  color text{ 0.92f, 0.94f, 0.98f, 1.0f };
  float padding_x = 8.0f;
  float padding_y = 4.0f;
  float rounding = 4.0f;
  float border_width = 1.0f;
};

/**
 * @brief Scroll view frame appearance
 *
 */
struct scroll_view_style {
  color background{ 0.07f, 0.09f, 0.13f, 1.0f };
  color border{ 0.24f, 0.30f, 0.38f, 1.0f };
  float rounding = 4.0f;
  float border_width = 1.0f;
};

/**
 * @brief Divider line appearance
 *
 */
struct divider_style {
  color color{ 0.36f, 0.42f, 0.50f, 1.0f };
};

/**
 * @brief Global appearance passed to widgets2::run
 *
 */
struct theme {
  float font_size = 0.0f; ///< Base font size; 0 uses Dear ImGui's current size
  text_style text;
  button_style button;
  choice_style checkbox;
  choice_style radio;
  text_input_style text_input;
  scroll_view_style scroll_view;
  divider_style divider;
};

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
