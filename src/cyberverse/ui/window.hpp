/**
 * @file window.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief UI window contract
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>
#include <optional>
#include <string_view>

namespace cyberverse {
namespace ui {

/**
 * @brief Rectangle in logical points
 *
 * The origin is the top-left corner of the window content area and +Y points
 * down.
 *
 */
struct rect {
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
};

/**
 * @brief Positive window features for one window
 *
 * The managed window has no title bar, no scrollbar, a background, input, and
 * no focus steal by default. Every flag opt-ins the corresponding Dear ImGui
 * behavior.
 *
 */
enum class window_feature : std::uint32_t {
  none = 0,
  title_bar = 1u << 0,               ///< Draw a title bar (window::title())
  scrollbar = 1u << 1,               ///< Allow a vertical scrollbar
  transparent = 1u << 2,             ///< Do not draw the window background
  no_input = 1u << 3,                ///< Display only, never receive input
  focus_on_appearing = 1u << 4,      ///< Allow the window to take focus when shown
  bring_to_front_on_focus = 1u << 5, ///< Allow focus to reorder the window in the system
};

/**
 * @brief Combine two window features
 *
 */
[[nodiscard]] constexpr window_feature operator|(window_feature a, window_feature b) noexcept {
  return static_cast<window_feature>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

/**
 * @brief Intersect two window features
 *
 */
[[nodiscard]] constexpr window_feature operator&(window_feature a, window_feature b) noexcept {
  return static_cast<window_feature>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

/**
 * @brief Whether every feature in @p flag is present in @p value
 *
 */
[[nodiscard]] constexpr bool has_feature(window_feature value, window_feature flag) noexcept {
  return (value & flag) == flag;
}

struct window_context;

/**
 * @brief One content window managed by a ui::system
 *
 * The system owns the Dear ImGui window and always sizes it to the system's
 * logical viewport. draw() only paints content; it must not open a top-level
 * window or set window position/size. Use Dear ImGui's native cursor, Dummy,
 * Spacing, child-window, and draw-list APIs to position content and whitespace.
 *
 * A window must return a stable, system-unique identifier. id(), title(),
 * visible(), and order() are called during a frame and must be cheap and
 * side-effect free.
 *
 */
class window {
public:
  virtual ~window() = default;

  /**
   * @brief Stable, system-unique window identifier
   *
   * Must not be empty and must not contain "###" or "/". The application is
   * responsible for system-wide uniqueness; it is validated when the window is
   * added.
   *
   * @return std::string_view
   */
  [[nodiscard]] virtual std::string_view id() const noexcept = 0;

  /**
   * @brief Optional window title; empty means no title
   *
   * The title may change, but it must not contain "###". The Dear ImGui window
   * ID stays stable through the "###<id>" suffix.
   *
   * @return std::string_view
   */
  [[nodiscard]] virtual std::string_view title() const noexcept { return {}; }

  /**
   * @brief Whether the window participates in this frame
   *
   * A hidden window is not drawn; other windows are unaffected.
   *
   * @return true
   * @return false
   */
  [[nodiscard]] virtual bool visible() const noexcept { return true; }

  /**
   * @brief Window feature flags
   *
   * @return window_feature
   */
  [[nodiscard]] virtual window_feature features() const noexcept { return window_feature::none; }

  /**
   * @brief Optional managed-window rectangle inside the system viewport
   *
   * The default is nullopt, which preserves the full-viewport managed window.
   * Return a finite positive rectangle to create a local window (for example a
   * sidebar). The system clamps the rectangle to the viewport; invalid values
   * fall back to the full viewport. This method is called during a frame and
   * must be cheap and side-effect free.
   *
   * @param viewport System viewport in logical points
   * @return std::optional<rect>
   */
  [[nodiscard]] virtual std::optional<rect> window_rect(const rect& viewport) const noexcept { return std::nullopt; }

  /**
   * @brief Draw order; higher values draw above lower values
   *
   * Equal orders keep insertion order. The default is 0.
   *
   */
  [[nodiscard]] virtual int order() const noexcept { return 0; }

  /**
   * @brief Draw the window's content inside the managed window
   *
   * Called between ImGui::Begin()/End() for a window that already covers the
   * whole logical viewport. The context is only valid during the call.
   *
   * @param context Frame information
   */
  virtual void draw(window_context& context) = 0;
};

} // namespace ui
} // namespace cyberverse
