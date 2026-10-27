/**
 * @file panel.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief UI panel contract
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>
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
 * @brief Positive window features for one panel
 *
 * The managed window has no title bar, no scrollbar, a background, input, and
 * no focus steal by default. Every flag opt-ins the corresponding Dear ImGui
 * behavior.
 *
 */
enum class panel_feature : std::uint32_t {
  none = 0,
  title_bar = 1u << 0,               ///< Draw a title bar (panel::title())
  scrollbar = 1u << 1,               ///< Allow a vertical scrollbar
  transparent = 1u << 2,             ///< Do not draw the window background
  no_input = 1u << 3,                ///< Display only, never receive input
  focus_on_appearing = 1u << 4,      ///< Allow the window to take focus when shown
  bring_to_front_on_focus = 1u << 5, ///< Allow focus to reorder the window inside its layer
};

/**
 * @brief Combine two panel features
 *
 */
[[nodiscard]] constexpr panel_feature operator|(panel_feature a, panel_feature b) noexcept {
  return static_cast<panel_feature>(static_cast<std::uint32_t>(a) | static_cast<std::uint32_t>(b));
}

/**
 * @brief Intersect two panel features
 *
 */
[[nodiscard]] constexpr panel_feature operator&(panel_feature a, panel_feature b) noexcept {
  return static_cast<panel_feature>(static_cast<std::uint32_t>(a) & static_cast<std::uint32_t>(b));
}

/**
 * @brief Whether every feature in @p flag is present in @p value
 *
 */
[[nodiscard]] constexpr bool has_feature(panel_feature value, panel_feature flag) noexcept {
  return (value & flag) == flag;
}

class panel_context;

/**
 * @brief One content unit managed by a ui::layer
 *
 * The layer owns the Dear ImGui window and always sizes it to the layer's
 * logical viewport. draw() only paints content; it must not open a top-level
 * window or set window position/size. Use Dear ImGui's native cursor, Dummy,
 * Spacing, child-window, and draw-list APIs to position content and whitespace.
 *
 * A panel must return stable, layer-unique identifiers. id(), title(), and
 * visible() are called during a frame and must be cheap and side-effect free.
 *
 */
class panel {
public:
  virtual ~panel() = default;

  /**
   * @brief Stable, layer-unique panel identifier
   *
   * Must not be empty and must not contain "###" or "/". The application is
   * responsible for layer-wide uniqueness; it is validated when the panel is
   * added.
   *
   * @return std::string_view
   */
  [[nodiscard]] virtual std::string_view id() const noexcept = 0;

  /**
   * @brief Optional window title; empty means no title
   *
   * The title may change, but it must not contain "###". The Dear ImGui window
   * ID stays stable through the "###<layer>/<id>" suffix.
   *
   * @return std::string_view
   */
  [[nodiscard]] virtual std::string_view title() const noexcept { return {}; }

  /**
   * @brief Whether the panel participates in this frame
   *
   * A hidden panel is not drawn; other panels are unaffected because the
   * managed window fills the whole layer viewport.
   *
   * @return true
   * @return false
   */
  [[nodiscard]] virtual bool visible() const noexcept { return true; }

  /**
   * @brief Window feature flags
   *
   * @return panel_feature
   */
  [[nodiscard]] virtual panel_feature features() const noexcept { return panel_feature::none; }

  /**
   * @brief Draw the panel's content inside the managed window
   *
   * Called between ImGui::Begin()/End() for a window that already covers the
   * whole logical viewport. The context is only valid during the call.
   *
   * @param context Frame and layer information
   */
  virtual void draw(panel_context& context) = 0;
};

} // namespace ui
} // namespace cyberverse
