/**
 * @file widgets_interaction_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Native widget interaction tests
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <memory>
#include <string>

#include <imgui.h>

#include "cyberverse/platform/backends/headless/window.hpp"
#include "cyberverse/platform/raw_input.hpp"
#include "cyberverse/ui/layer.hpp"
#include "cyberverse/ui/panel.hpp"
#include "cyberverse/ui/panel_context.hpp"
#include "cyberverse/ui/system.hpp"
#include "cyberverse/ui/widgets.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::layer;
using cyberverse::ui::layer_desc;
using cyberverse::ui::panel_context;
using cyberverse::ui::system;
using cyberverse::ui::test::make_headless;
using cyberverse::ui::widgets::button;
using cyberverse::ui::widgets::checkbox;
using cyberverse::ui::widgets::flexbox;
using cyberverse::ui::widgets::radio;

constexpr float width = 320.0f;
constexpr float height = 240.0f;
constexpr double frame_delta = 1.0 / 60.0;

class widget_panel final : public cyberverse::ui::panel {
public:
  explicit widget_panel(std::string id, std::function<void()> on_draw)
      : id_{ std::move(id) }, on_draw_{ std::move(on_draw) } {}

  [[nodiscard]] std::string_view id() const noexcept override { return id_; }
  [[nodiscard]] bool visible() const noexcept override { return true; }

  void draw(panel_context& /*context*/) override {
    ImGui::SetCursorScreenPos(ImGui::GetWindowPos());
    on_draw_();
  }

private:
  std::string id_;
  std::function<void()> on_draw_;
};

void run_frame(system& ui) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

void click(cyberverse::platform::backends::headless::window& window, system& ui, const cyberverse::ui::rect& target) {
  const auto left = cyberverse::platform::mouse_button::left;
  window.inject_mouse_move(target.x + target.width * 0.5f, target.y + target.height * 0.5f);
  window.inject_mouse_button_down(left);
  window.poll_events();
  run_frame(ui);
  window.inject_mouse_button_up(left);
  window.poll_events();
  run_frame(ui);
}

} // namespace

TEST_CASE("button invokes its callback when clicked", "[ui][widgets][interaction]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));

  int clicks = 0;
  cyberverse::ui::rect button_bounds{};
  auto panel = std::make_unique<widget_panel>("widgets.button", [&] {
    flexbox root;
    root.size(160.0f, 80.0f).padding(cyberverse::ui::widgets::insets{ 10, 10, 10, 10 });
    auto& control = root.add(button{
        "OK", [&] {
          ++clicks;
        } }.width_stretch());
    cyberverse::ui::widgets::render(root);
    button_bounds = control.bounds();
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  click(context.headless_window(), *context.ui, button_bounds);

  REQUIRE(clicks == 1);
}

TEST_CASE("checkbox toggles its external value", "[ui][widgets][interaction]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));

  bool checked = false;
  cyberverse::ui::rect checkbox_bounds{};
  auto panel = std::make_unique<widget_panel>("widgets.checkbox", [&] {
    flexbox root;
    root.size(200.0f, 80.0f);
    auto& control = root.add(checkbox{ "Enable", checked });
    cyberverse::ui::widgets::render(root);
    checkbox_bounds = control.bounds();
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  click(context.headless_window(), *context.ui, checkbox_bounds);

  REQUIRE(checked);
}

TEST_CASE("radio sets the shared selection value", "[ui][widgets][interaction]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));

  int selected = 0;
  cyberverse::ui::rect first_bounds{};
  cyberverse::ui::rect second_bounds{};
  auto panel = std::make_unique<widget_panel>("widgets.radio", [&] {
    flexbox root;
    root.size(200.0f, 120.0f).gap(4.0f);
    auto& first = root.add(radio{ "One", selected, 0 });
    auto& second = root.add(radio{ "Two", selected, 1 });
    cyberverse::ui::widgets::render(root);
    first_bounds = first.bounds();
    second_bounds = second.bounds();
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  click(context.headless_window(), *context.ui, second_bounds);
  REQUIRE(selected == 1);

  click(context.headless_window(), *context.ui, first_bounds);
  REQUIRE(selected == 0);
}
