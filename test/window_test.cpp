/**
 * @file window_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Managed UI window tests
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include <imgui_internal.h>

#include "cyberverse/ui/layer.hpp"
#include "cyberverse/ui/system.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::layer;
using cyberverse::ui::layer_desc;
using cyberverse::ui::system;
using cyberverse::ui::window_context;
using cyberverse::ui::window_feature;
using cyberverse::ui::test::fake_window;
using cyberverse::ui::test::make_headless;
using cyberverse::ui::test::window_name;

constexpr float width = 320.0f;
constexpr float height = 200.0f;
constexpr double frame_delta = 1.0 / 60.0;

layer& add_layer(system& ui, std::string name) {
  return ui.add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = std::move(name) }));
}

void render(system& ui) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

} // namespace

TEST_CASE("managed windows fill the layer viewport", "[ui][window]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = add_layer(*context.ui, "app");
  auto content = std::make_unique<fake_window>();
  fake_window* content_ptr = content.get();
  layer.add_window(std::move(content));

  render(*context.ui);

  REQUIRE(content_ptr->draw_count == 1);
  REQUIRE(content_ptr->last_bounds.x == 0.0f);
  REQUIRE(content_ptr->last_bounds.y == 0.0f);
  REQUIRE(content_ptr->last_bounds.width == width);
  REQUIRE(content_ptr->last_bounds.height == height);
  REQUIRE(content_ptr->last_window_pos.x == 0.0f);
  REQUIRE(content_ptr->last_window_pos.y == 0.0f);
  REQUIRE(content_ptr->last_window_size.x == width);
  REQUIRE(content_ptr->last_window_size.y == height);
}

TEST_CASE("managed windows honor a requested local rect", "[ui][window]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = add_layer(*context.ui, "app");
  auto content = std::make_unique<fake_window>();
  content->rect_value = cyberverse::ui::rect{ 40.0f, 30.0f, 120.0f, 100.0f };
  fake_window* content_ptr = content.get();
  layer.add_window(std::move(content));

  render(*context.ui);

  REQUIRE(content_ptr->last_bounds.x == 40.0f);
  REQUIRE(content_ptr->last_bounds.y == 30.0f);
  REQUIRE(content_ptr->last_bounds.width == 120.0f);
  REQUIRE(content_ptr->last_bounds.height == 100.0f);
  REQUIRE(content_ptr->last_window_pos.x == 40.0f);
  REQUIRE(content_ptr->last_window_pos.y == 30.0f);
  REQUIRE(content_ptr->last_window_size.x == 120.0f);
  REQUIRE(content_ptr->last_window_size.y == 100.0f);
}

TEST_CASE("local windows only capture the mouse inside their rect", "[ui][window]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = add_layer(*context.ui, "app");
  auto content = std::make_unique<fake_window>();
  content->rect_value = cyberverse::ui::rect{ 200.0f, 50.0f, 100.0f, 100.0f };
  content->on_draw = [](window_context&) {
    ImGui::Button("local", ImVec2(100.0f, 100.0f));
  };
  layer.add_window(std::move(content));

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

TEST_CASE("multiple windows in one layer draw in add order", "[ui][window]") {
  auto context = make_headless();
  auto& layer = add_layer(*context.ui, "app");
  std::vector<std::string> draw_order;

  for (const char* id : { "first", "second", "third" }) {
    auto content = std::make_unique<fake_window>();
    content->id_value = id;
    const std::string captured = id;
    content->on_draw = [&draw_order, captured](window_context&) {
      draw_order.push_back(captured);
    };
    layer.add_window(std::move(content));
  }

  render(*context.ui);
  REQUIRE(draw_order == std::vector<std::string>{ "first", "second", "third" });
}

TEST_CASE("no_input windows do not capture the mouse", "[ui][window]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = add_layer(*context.ui, "app");
  auto content = std::make_unique<fake_window>();
  content->feature_value = window_feature::no_input | window_feature::transparent;
  layer.add_window(std::move(content));

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
  auto& layer = add_layer(*context.ui, "app");
  layer.add_window(std::make_unique<fake_window>());

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
  auto& layer = add_layer(*context.ui, "app");
  auto content = std::make_unique<fake_window>();
  fake_window* content_ptr = content.get();
  content_ptr->throw_on_draw = true;
  layer.add_window(std::move(content));

  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  REQUIRE_THROWS_AS(context.ui->draw(), std::runtime_error);
  REQUIRE_FALSE(context.ui->frame_active());

  content_ptr->throw_on_draw = false;
  render(*context.ui);
  REQUIRE(content_ptr->draw_count == 2);
}

TEST_CASE("window names stay stable when the title changes", "[ui][window]") {
  auto context = make_headless();
  auto& layer = add_layer(*context.ui, "app");
  auto content = std::make_unique<fake_window>();
  fake_window* content_ptr = content.get();
  content_ptr->title_value = "First";
  layer.add_window(std::move(content));

  render(*context.ui);
  const std::string expected = window_name("app", "test.window", "First");
  REQUIRE(ImGui::FindWindowByName(expected.c_str()) != nullptr);

  content_ptr->title_value = "Second";
  render(*context.ui);
  const std::string renamed = window_name("app", "test.window", "Second");
  REQUIRE(ImGui::FindWindowByName(renamed.c_str()) != nullptr);
}
