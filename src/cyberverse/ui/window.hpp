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
#include <string>
#include <string_view>

#include "cyberverse/ui/rect.hpp"

namespace cyberverse {
namespace ui {

/**
 * @brief Positive window features for one window
 *
 * The managed window has no scrollbar, a background, input, and no focus
 * steal by default. Every flag opt-ins the corresponding Dear ImGui behavior.
 *
 */
enum class window_feature : std::uint32_t {
  none = 0,
  scrollbar = 1u << 0,               ///< Allow a vertical scrollbar
  transparent = 1u << 1,             ///< Do not draw the window background
  no_input = 1u << 2,                ///< Display only, never receive input
  focus_on_appearing = 1u << 3,      ///< Allow the window to take focus when shown
  bring_to_front_on_focus = 1u << 4, ///< Allow focus to reorder the window in the host
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
 * @brief One content window managed by a ui::host
 *
 * The host owns the Dear ImGui window and always sizes it to the host's
 * logical viewport. draw() only paints content; it must not open a top-level
 * window or set window position/size. Use Dear ImGui's native cursor, Dummy,
 * Spacing, child-window, and draw-list APIs to position content and whitespace.
 *
 * A window has one stable, host-unique identifier supplied at construction.
 * visible() and order() are called during a frame and must be cheap and
 * side-effect free.
 *
 */
class window {
public:
  virtual ~window() = default;

  /**
   * @brief Stable, host-unique window identifier
   *
   * The identifier is stored as a std::string and exposed as a view. Its data
   * is null-terminated because it is also passed directly to Dear ImGui as the
   * window name. It must not be empty, must not contain "###", and must not
   * contain an embedded null character.
   *
   * @return std::string_view
   */
  [[nodiscard]] std::string_view id() const noexcept { return id_; }

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
   * @brief Optional managed-window rectangle inside the host viewport
   *
   * The default is nullopt, which preserves the full-viewport managed window.
   * Return a finite positive rectangle to create a local window (for example a
   * sidebar). The host clamps the rectangle to the viewport; invalid values
   * fall back to the full viewport. This method is called during a frame and
   * must be cheap and side-effect free.
   *
   * @param viewport Host viewport in logical points
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

protected:
  /**
   * @brief Construct a window with its stable Dear ImGui identifier
   *
   * @param id Stable identifier; std::string guarantees a trailing '\0'
   * @throws std::invalid_argument If id is empty, contains "###", or contains
   *         an embedded null character
   */
  explicit window(std::string id);

private:
  std::string id_;
};

} // namespace ui
} // namespace cyberverse
