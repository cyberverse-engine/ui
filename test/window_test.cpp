/**
 * @file window_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Managed UI window tests
 * @version 0.4
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <imgui_internal.h>

#include "cyberverse/ui/host.hpp"
#include "cyberverse/ui/window.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::host;
using cyberverse::ui::window_context;
using cyberverse::ui::window_feature;
using cyberverse::ui::test::fake_window;
using cyberverse::ui::test::make_headless;

constexpr float width = 320.0f;
constexpr float height = 200.0f;
constexpr double frame_delta = 1.0 / 60.0;

fake_window& add_fake(host& ui, std::string id) {
  auto content = std::make_unique<fake_window>(std::move(id));
  return static_cast<fake_window&>(ui.add_window(std::move(content)));
}

void render(host& ui) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

} // namespace

TEST_CASE("managed windows fill the system viewport", "[ui][window]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  fake_window& content = add_fake(*context.ui, "test.viewport");

  render(*context.ui);

  REQUIRE(content.draw_count == 1);
  REQUIRE(content.last_bounds.x == 0.0f);
  REQUIRE(content.last_bounds.y == 0.0f);
  REQUIRE(content.last_bounds.width == width);
  REQUIRE(content.last_bounds.height == height);
  REQUIRE(content.last_window_pos.x == 0.0f);
  REQUIRE(content.last_window_pos.y == 0.0f);
  REQUIRE(content.last_window_size.x == width);
  REQUIRE(content.last_window_size.y == height);
}

TEST_CASE("managed windows honor a requested local rect", "[ui][window]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  fake_window& content = add_fake(*context.ui, "test.local");
  content.rect_value = cyberverse::ui::rect{ 40.0f, 30.0f, 120.0f, 100.0f };

  render(*context.ui);

  REQUIRE(content.last_bounds.x == 40.0f);
  REQUIRE(content.last_bounds.y == 30.0f);
  REQUIRE(content.last_bounds.width == 120.0f);
  REQUIRE(content.last_bounds.height == 100.0f);
  REQUIRE(content.last_window_pos.x == 40.0f);
  REQUIRE(content.last_window_pos.y == 30.0f);
  REQUIRE(content.last_window_size.x == 120.0f);
  REQUIRE(content.last_window_size.y == 100.0f);
}

TEST_CASE("local windows only capture the mouse inside their rect", "[ui][window]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  fake_window& content = add_fake(*context.ui, "test.capture");
  content.rect_value = cyberverse::ui::rect{ 200.0f, 50.0f, 100.0f, 100.0f };
  content.on_draw = [](window_context&) {
    ImGui::Button("local", ImVec2(100.0f, 100.0f));
  };

  auto& window = context.headless_window();
  window.inject_mouse_move(50.0f, 50.0f);
  window.poll_events();
  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  context.ui->draw();
  context.ui->end_frame();
  REQUIRE_FALSE(context.ui->wants_mouse());

  window.inject_mouse_move(250.0f, 100.0f);
  window.poll_events();
  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  context.ui->draw();
  context.ui->end_frame();
  REQUIRE(context.ui->wants_mouse());
}

TEST_CASE("multiple windows draw in insertion order on equal order", "[ui][window]") {
  auto context = make_headless();
  std::vector<std::string> draw_order;

  for (const char* id : { "first", "second", "third" }) {
    auto content = std::make_unique<fake_window>(id);
    const std::string captured = id;
    content->on_draw = [&draw_order, captured](window_context&) {
      draw_order.push_back(captured);
    };
    context.ui->add_window(std::move(content));
  }

  render(*context.ui);
  REQUIRE(draw_order == std::vector<std::string>{ "first", "second", "third" });
}

TEST_CASE("no_input windows do not capture the mouse", "[ui][window]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  fake_window& content = add_fake(*context.ui, "test.noinput");
  content.feature_value = window_feature::no_input | window_feature::transparent;

  render(*context.ui);

  auto& window = context.headless_window();
  window.inject_mouse_move(width * 0.5f, height * 0.5f);
  window.poll_events();
  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  context.ui->end_frame();

  REQUIRE_FALSE(context.ui->wants_mouse());
}

TEST_CASE("interactive windows capture the mouse", "[ui][window]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  add_fake(*context.ui, "test.interactive");

  render(*context.ui);

  auto& window = context.headless_window();
  window.inject_mouse_move(width * 0.5f, height * 0.5f);
  window.poll_events();
  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  context.ui->end_frame();

  REQUIRE(context.ui->wants_mouse());
}

TEST_CASE("a throwing window cleans up the frame and the next frame recovers", "[ui][window]") {
  auto context = make_headless();
  fake_window& content = add_fake(*context.ui, "test.throwing");
  content.throw_on_draw = true;

  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  REQUIRE_THROWS_AS(context.ui->draw(), std::runtime_error);
  REQUIRE_FALSE(context.ui->frame_active());

  content.throw_on_draw = false;
  render(*context.ui);
  REQUIRE(content.draw_count == 2);
}

TEST_CASE("window context preserves the logical frame delta", "[ui][window][clock]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  fake_window& content = add_fake(*context.ui, "test.clock");
  double observed_delta = -1.0;
  content.on_draw = [&observed_delta](window_context& window_context) {
    observed_delta = window_context.delta_time;
  };

  REQUIRE(context.ui->begin_frame(width, height, 0.25));
  context.ui->draw();
  context.ui->end_frame();
  REQUIRE(observed_delta == Catch::Approx(0.25));

  REQUIRE(context.ui->begin_frame(width, height, 0.0));
  context.ui->draw();
  context.ui->end_frame();
  REQUIRE(observed_delta == Catch::Approx(0.0));
  REQUIRE(ImGui::GetIO().DeltaTime > 0.0f);

  REQUIRE(context.ui->begin_frame(width, height, -1.0));
  context.ui->draw();
  context.ui->end_frame();
  REQUIRE(observed_delta == Catch::Approx(0.0));
}

TEST_CASE("the window id is the Dear ImGui window name", "[ui][window]") {
  auto context = make_headless();
  fake_window& content = add_fake(*context.ui, "test.window");

  render(*context.ui);
  REQUIRE(ImGui::FindWindowByName("test.window") != nullptr);
  REQUIRE(content.id() == "test.window");
}
