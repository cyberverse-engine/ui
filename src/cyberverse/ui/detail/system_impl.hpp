/**
 * @file system_impl.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief ui::system implementation details
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
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include <ImGuiDiligentRenderer.hpp>
#include <imgui.h>

#include "cyberverse/ui/detail/input_bridge.hpp"
#include "cyberverse/ui/system.hpp"
#include "cyberverse/ui/window.hpp"

namespace cyberverse {
namespace ui {

/**
 * @brief Private implementation of ui::system
 *
 * Holds the single Dear ImGui context, the Diligent ImGui renderer, the input
 * bridge, the font cache, and the ordered window list.
 *
 */
struct system::impl {
  explicit impl(const system_config& config);
  ~impl();

  impl(const impl&) = delete;
  impl(impl&&) = delete;
  impl& operator=(const impl&) = delete;
  impl& operator=(impl&&) = delete;

  window& add_window(std::unique_ptr<window> content);
  bool remove_window(window& content) noexcept;
  void clear_windows();
  [[nodiscard]] window* find_window(std::string_view id) noexcept;
  [[nodiscard]] bool has_visible_content() const noexcept;

  bool begin_frame(const render_core::frame_context& frame, double delta_time);
  bool begin_frame(float display_width, float display_height, double delta_time);
  void draw(const system& owner);
  void end_frame();

  [[nodiscard]] bool frame_active() const noexcept { return frame_active_; }
  [[nodiscard]] bool wants_mouse() const noexcept { return wants_mouse_; }
  [[nodiscard]] bool wants_keyboard() const noexcept { return wants_keyboard_; }

  void reload_fonts();
  [[nodiscard]] bool font_loaded(std::string_view id) const noexcept;
  void set_theme(theme_kind theme);
  void set_scale(float scale);
  [[nodiscard]] float scale() const noexcept { return scale_; }

  [[nodiscard]] ImGuiContext* imgui_context() noexcept { return context_; }
  [[nodiscard]] ImGuiIO& io() noexcept;
  [[nodiscard]] ImGuiStyle& style() noexcept;

private:
  void create_context();
  void install_ime_hook();
  void apply_theme(theme_kind theme);
  void load_fonts();
  [[nodiscard]] const ImWchar* ranges_for(const font_config& config);
  void update_input();
  void rebuild_draw_order();
  [[nodiscard]] ImGuiWindowFlags window_flags(const window& content) const;
  void build_window_name(const window& content);
  void abort_frame() noexcept;
  void destroy_context() noexcept;

  system_config config_;
  ImGuiContext* context_ = nullptr;
  std::unique_ptr<Diligent::ImGuiDiligentRenderer> renderer_;
  std::string ini_path_;
  std::string window_name_;
  std::vector<std::string> window_names_;
  std::vector<std::unique_ptr<window>> windows_;
  std::vector<window*> draw_order_;
  std::vector<std::pair<window*, int>> window_signature_;
  std::unordered_map<std::string, bool> font_loaded_;
  std::vector<std::vector<ImWchar>> custom_ranges_;
  detail::input_bridge input_;
  Diligent::IDeviceContext* device_context_ = nullptr;

  render_core::render_device* device_ = nullptr;
  platform::window* window_ = nullptr;

  bool headless_ = false;
  bool frame_active_ = false;
  bool manual_scale_ = false;
  bool wants_mouse_ = false;
  bool wants_keyboard_ = false;

  float scale_ = 1.0f;
  float display_width_ = 0.0f;
  float display_height_ = 0.0f;
  std::uint64_t frame_index_ = 0;
  double frame_delta_ = 0.0;
};

} // namespace ui
} // namespace cyberverse
