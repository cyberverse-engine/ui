/**
 * @file widgets_text_input_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Text input and IME integration tests
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <memory>
#include <string>
#include <utility>

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
using cyberverse::ui::widgets::flexbox;
using cyberverse::ui::widgets::text_input;

constexpr float width = 320.0f;
constexpr float height = 200.0f;
constexpr double frame_delta = 1.0 / 60.0;

class input_panel final : public cyberverse::ui::panel {
public:
  input_panel(std::string id, std::string& value) : id_{ std::move(id) }, value_{ value } {}

  [[nodiscard]] std::string_view id() const noexcept override { return id_; }
  [[nodiscard]] bool visible() const noexcept override { return true; }

  void draw(panel_context& /*context*/) override {
    ImGui::SetCursorScreenPos(ImGui::GetWindowPos());
    flexbox root;
    root.size(300.0f, 120.0f).padding(cyberverse::ui::widgets::insets{ 12, 12, 12, 12 }).gap(8.0f);
    auto& input = root.add(text_input{ "player.name", value_ }.placeholder("Player name"));
    auto& label = root.add(cyberverse::ui::widgets::text{ "Name" });
    cyberverse::ui::widgets::render(root);
    input_bounds_ = input.bounds();
    label_bounds_ = label.bounds();
  }

  cyberverse::ui::rect input_bounds_{};
  cyberverse::ui::rect label_bounds_{};

private:
  std::string id_;
  std::string& value_;
};

void run_frame(system& ui) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

} // namespace

TEST_CASE("text input accepts committed UTF-8 text", "[ui][widgets][input]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  context.ui->io().ConfigInputTrickleEventQueue = false;

  std::string value;
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));
  auto panel = std::make_unique<input_panel>("widgets.input", value);
  input_panel* panel_ptr = panel.get();
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);

  auto& window = context.headless_window();
  const cyberverse::platform::mouse_button left = cyberverse::platform::mouse_button::left;
  window.inject_mouse_move(panel_ptr->input_bounds_.x + panel_ptr->input_bounds_.width * 0.5f,
                           panel_ptr->input_bounds_.y + panel_ptr->input_bounds_.height * 0.5f);
  window.inject_mouse_button_down(left);
  window.poll_events();
  run_frame(*context.ui);

  window.inject_mouse_button_up(left);
  window.poll_events();
  run_frame(*context.ui);

  window.inject_text("中文");
  window.poll_events();
  run_frame(*context.ui);

  REQUIRE(value == "中文");
  REQUIRE(panel_ptr->input_bounds_.width > 0.0f);
  REQUIRE(panel_ptr->input_bounds_.height > 0.0f);
}

TEST_CASE("focused text input reports an IME caret", "[ui][widgets][input][ime]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  context.ui->io().ConfigInputTrickleEventQueue = false;

  std::string value;
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));
  auto panel = std::make_unique<input_panel>("widgets.ime", value);
  input_panel* panel_ptr = panel.get();
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);

  auto& window = context.headless_window();
  const cyberverse::platform::mouse_button left = cyberverse::platform::mouse_button::left;
  window.inject_mouse_move(panel_ptr->input_bounds_.x + panel_ptr->input_bounds_.width * 0.5f,
                           panel_ptr->input_bounds_.y + panel_ptr->input_bounds_.height * 0.5f);
  window.inject_mouse_button_down(left);
  window.poll_events();
  run_frame(*context.ui);

  window.inject_mouse_button_up(left);
  window.poll_events();
  run_frame(*context.ui);

  const auto caret = window.last_ime_caret();
  REQUIRE(caret.visible);
  REQUIRE(caret.height > 0.0f);
  REQUIRE(caret.x >= panel_ptr->input_bounds_.x - 1.0f);
  REQUIRE(caret.x <= panel_ptr->input_bounds_.x + panel_ptr->input_bounds_.width + 1.0f);
  REQUIRE(caret.y >= panel_ptr->input_bounds_.y - 1.0f);
  REQUIRE(caret.y <= panel_ptr->input_bounds_.y + panel_ptr->input_bounds_.height + 1.0f);
}
