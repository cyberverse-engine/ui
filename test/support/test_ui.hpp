/**
 * @file test_ui.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Shared helpers for the cyberverse::ui tests
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <string_view>
#include <utility>

#include "cyberverse/platform/backends/headless/window.hpp"
#include "cyberverse/platform/platform.hpp"
#include "cyberverse/platform/window.hpp"
#include "cyberverse/ui/imgui.hpp"
#include "cyberverse/ui/panel.hpp"
#include "cyberverse/ui/panel_context.hpp"
#include "cyberverse/ui/system.hpp"

namespace cyberverse {
namespace ui {
namespace test {

/**
 * @brief Headless platform + window + UI system bundle
 *
 */
struct headless_context {
  std::shared_ptr<platform::platform> platform;
  std::shared_ptr<platform::window> window;
  std::unique_ptr<system> ui;

  [[nodiscard]] platform::backends::headless::window& headless_window() const {
    return *std::static_pointer_cast<platform::backends::headless::window>(window);
  }
};

inline headless_context make_headless(int width = 640, int height = 480, bool with_window = true) {
  headless_context context;
  context.platform = platform::make_platform(platform::backend_type::headless);
  if (with_window) {
    platform::window_config window_config;
    window_config.width = width;
    window_config.height = height;
    window_config.title = "cyberverse-ui-test";
    context.window = context.platform->create_window(window_config);
  }

  system_config ui_config;
  ui_config.headless = true;
  ui_config.window = context.window.get();
  ui_config.fonts = { font_config{ .id = "default", .size = 16.0f } };
  context.ui = system::create(ui_config);
  return context;
}

/**
 * @brief Recording panel used by most tests
 *
 */
class fake_panel final : public panel {
public:
  std::string id_value = "test.panel";
  std::string title_value;
  bool visible_value = true;
  panel_feature feature_value = panel_feature::none;
  bool throw_on_draw = false;
  int draw_count = 0;
  rect last_bounds{};
  ImVec2 last_window_pos{};
  ImVec2 last_window_size{};
  std::function<void(panel_context&)> on_draw;

  [[nodiscard]] std::string_view id() const noexcept override { return id_value; }
  [[nodiscard]] std::string_view title() const noexcept override { return title_value; }
  [[nodiscard]] bool visible() const noexcept override { return visible_value; }
  [[nodiscard]] panel_feature features() const noexcept override { return feature_value; }

  void draw(panel_context& context) override {
    ++draw_count;
    last_bounds = context.bounds;
    last_window_pos = ImGui::GetWindowPos();
    last_window_size = ImGui::GetWindowSize();
    if (on_draw) {
      on_draw(context);
    }
    if (throw_on_draw) {
      throw std::runtime_error("panel draw failed");
    }
  }
};

[[nodiscard]] inline std::string window_name(std::string_view layer, std::string_view id, std::string_view title = {}) {
  std::string name;
  name.append(title.data(), title.size());
  name += "###";
  name.append(layer.data(), layer.size());
  name += '/';
  name.append(id.data(), id.size());
  return name;
}

} // namespace test
} // namespace ui
} // namespace cyberverse
