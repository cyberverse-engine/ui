/**
 * @file system.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Dear ImGui + DiligentEngine UI runtime
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string_view>
#include <vector>

#include "cyberverse/ui/font.hpp"
#include "cyberverse/ui/theme.hpp"

struct ImGuiContext;
struct ImGuiIO;
struct ImGuiStyle;

namespace cyberverse {

namespace platform {
class window;
} // namespace platform

namespace render_core {
class frame_context;
class render_device;
} // namespace render_core

namespace ui {

class frame_stage;

/**
 * @brief Creation description for ui::system
 *
 * headless == true skips Diligent rendering, creates no GPU resources, and
 * produces draw data only. Production rendering requires both device and
 * window. A headless system may still own a platform headless window to feed
 * deterministic input.
 *
 */
struct system_config {
  bool headless = false;                        ///< Skip GPU rendering
  render_core::render_device* device = nullptr; ///< Required unless headless
  platform::window* window = nullptr;           ///< Required unless headless
  std::vector<font_config> fonts;               ///< First font is the default
  theme_kind theme = theme_kind::dark;          ///< Initial style preset
  bool keyboard_nav = true;                     ///< Enable ImGui keyboard navigation
  bool persist_ini = false;                     ///< Persist ImGui window settings
  std::filesystem::path ini_path;               ///< INI file when persist_ini is true
  bool enable_dpi_scaling = true;               ///< Follow window::framebuffer_scale()
};

/**
 * @brief The application-facing UI facade
 *
 * system owns the single Dear ImGui context, the Diligent ImGui renderer, the
 * input bridge, fonts, theme, and the ordered layer stack. One engine frame
 * opens at most one Dear ImGui frame, and every layer shares it.
 *
 * Lifecycle:
 * - create() creates the ImGui context first, then fonts and the renderer. Any
 *   failure cleans up what was already created and throws.
 * - The system must be destroyed before the render device and platform window.
 *   Remove the ui::runtime_stage from the runtime before destroying it.
 * - begin_frame() returns false when no drawable frame exists; draw() and
 *   end_frame() must not be called then. end_frame() without an active frame is
 *   a no-op.
 * - Layer and panel structure may only change between frames; changing it during
 *   a frame is a std::logic_error.
 * - All methods must be called from the render thread. system is not
 *   thread-safe.
 *
 */
class system {
public:
  /**
   * @brief Create the UI runtime
   *
   * @param config Creation description
   * @return std::unique_ptr<system>
   * @throws std::invalid_argument If a non-headless config lacks device/window
   * @throws std::runtime_error If the ImGui context or Diligent renderer fails
   */
  [[nodiscard]] static std::unique_ptr<system> create(const system_config& config);

  ~system();

  system(const system&) = delete;
  system(system&&) = delete;
  system& operator=(const system&) = delete;
  system& operator=(system&&) = delete;

  /**
   * @brief Add an owned UI layer
   *
   * Layer names must be unique, non-empty, and contain neither "###" nor "/".
   *
   * @throws std::invalid_argument On an invalid or duplicate name
   * @throws std::logic_error During an active frame
   */
  frame_stage& add_layer(std::unique_ptr<frame_stage> layer);

  /**
   * @brief Remove a layer owned by the system
   *
   * @return true when the layer was owned and removed
   */
  bool remove_layer(frame_stage& layer) noexcept;

  /**
   * @brief Remove every layer
   *
   * @throws std::logic_error During an active frame
   */
  void clear_layers();

  /**
   * @brief Find a layer by name
   *
   * @return frame_stage* or null
   */
  [[nodiscard]] frame_stage* find_layer(std::string_view name) noexcept;

  /**
   * @brief Whether any visible layer has a visible panel
   *
   */
  [[nodiscard]] bool has_visible_content() const noexcept;

  /**
   * @brief Start a frame for the render path
   *
   * Uses the window framebuffer size and framebuffer scale when a window is
   * present, otherwise the frame context viewport. Requires a valid device
   * context and color target in the non-headless mode.
   *
   * @param frame Render frame context; must outlive the frame
   * @param delta_time Frame delta in seconds
   * @return true when a frame started
   * @return false when no drawable frame exists or a frame is already active
   */
  bool begin_frame(const render_core::frame_context& frame, double delta_time);

  /**
   * @brief Start a headless frame with an explicit logical size
   *
   * Only valid for a headless system.
   *
   * @param display_width Logical width in points; must be positive
   * @param display_height Logical height in points; must be positive
   * @param delta_time Frame delta in seconds
   * @return true when a frame started
   */
  bool begin_frame(float display_width, float display_height, double delta_time);

  /**
   * @brief Draw every visible layer inside the active Dear ImGui frame
   *
   * No-op when no frame is active.
   *
   */
  void draw();

  /**
   * @brief Submit the Dear ImGui frame and the overlay pass
   *
   * No-op when no frame is active.
   *
   */
  void end_frame();

  /**
   * @brief Whether begin_frame() succeeded and end_frame() has not run
   *
   */
  [[nodiscard]] bool frame_active() const noexcept;

  /**
   * @brief Global mouse capture from the most recently completed frame
   *
   * Applications read this on the next frame, so it has one frame of latency.
   *
   */
  [[nodiscard]] bool wants_mouse() const noexcept;

  /**
   * @brief Global keyboard capture from the most recently completed frame
   *
   */
  [[nodiscard]] bool wants_keyboard() const noexcept;

  /**
   * @brief Rebuild the font atlas between frames
   *
   * @throws std::logic_error During an active frame
   */
  void reload_fonts();

  /**
   * @brief Whether a configured font id loaded successfully
   *
   */
  [[nodiscard]] bool font_loaded(std::string_view id) const noexcept;

  /**
   * @brief Apply a theme preset between frames
   *
   * @throws std::logic_error During an active frame
   */
  void set_theme(theme_kind theme);

  /**
   * @brief Override the logical-to-physical scale between frames
   *
   * @throws std::invalid_argument If scale is not finite or not positive
   * @throws std::logic_error During an active frame
   */
  void set_scale(float scale);

  /**
   * @brief Current logical-to-physical scale
   *
   */
  [[nodiscard]] float scale() const noexcept;

  /**
   * @brief Explicit escape hatch: the owned Dear ImGui context
   *
   */
  [[nodiscard]] ImGuiContext* imgui_context() noexcept;

  /**
   * @brief Explicit escape hatch: the Dear ImGui IO
   *
   * Include "cyberverse/ui/imgui.hpp" to use the returned type.
   *
   */
  [[nodiscard]] ImGuiIO& io() noexcept;

  /**
   * @brief Explicit escape hatch: the Dear ImGui style
   *
   */
  [[nodiscard]] ImGuiStyle& style() noexcept;

private:
  struct impl;

  explicit system(std::unique_ptr<impl> impl);

  std::unique_ptr<impl> impl_;
};

} // namespace ui
} // namespace cyberverse
