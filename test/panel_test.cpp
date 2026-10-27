/**
 * @file panel_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Managed panel window tests
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

#include "cyberverse/ui/frame_stage.hpp"
#include "cyberverse/ui/system.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::frame_stage;
using cyberverse::ui::frame_stage_desc;
using cyberverse::ui::panel_context;
using cyberverse::ui::panel_feature;
using cyberverse::ui::system;
using cyberverse::ui::test::fake_panel;
using cyberverse::ui::test::make_headless;
using cyberverse::ui::test::window_name;

constexpr float width = 320.0f;
constexpr float height = 200.0f;
constexpr double frame_delta = 1.0 / 60.0;

frame_stage& add_layer(system& ui, std::string name) {
  return ui.add_layer(std::make_unique<frame_stage>(frame_stage_desc{ .name = std::move(name) }));
}

void render(system& ui) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

} // namespace

TEST_CASE("managed panel windows fill the layer viewport", "[ui][panel]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = add_layer(*context.ui, "app");
  auto content = std::make_unique<fake_panel>();
  fake_panel* content_ptr = content.get();
  layer.add_panel(std::move(content));

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

TEST_CASE("multiple panels in one layer draw in add order", "[ui][panel]") {
  auto context = make_headless();
  auto& layer = add_layer(*context.ui, "app");
  std::vector<std::string> draw_order;

  for (const char* id : { "first", "second", "third" }) {
    auto content = std::make_unique<fake_panel>();
    content->id_value = id;
    const std::string captured = id;
    content->on_draw = [&draw_order, captured](panel_context&) {
      draw_order.push_back(captured);
    };
    layer.add_panel(std::move(content));
  }

  render(*context.ui);
  REQUIRE(draw_order == std::vector<std::string>{ "first", "second", "third" });
}

TEST_CASE("no_input panels do not capture the mouse", "[ui][panel]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = add_layer(*context.ui, "app");
  auto content = std::make_unique<fake_panel>();
  content->feature_value = panel_feature::no_input | panel_feature::transparent;
  layer.add_panel(std::move(content));

  render(*context.ui);

  auto& window = context.headless_window();
  window.inject_mouse_move(width * 0.5f, height * 0.5f);
  window.poll_events();
  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  context.ui->end_frame();

  REQUIRE_FALSE(context.ui->wants_mouse());
}

TEST_CASE("interactive panels capture the mouse", "[ui][panel]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = add_layer(*context.ui, "app");
  layer.add_panel(std::make_unique<fake_panel>());

  render(*context.ui);

  auto& window = context.headless_window();
  window.inject_mouse_move(width * 0.5f, height * 0.5f);
  window.poll_events();
  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  context.ui->end_frame();

  REQUIRE(context.ui->wants_mouse());
}

TEST_CASE("a throwing panel cleans up the frame and the next frame recovers", "[ui][panel]") {
  auto context = make_headless();
  auto& layer = add_layer(*context.ui, "app");
  auto content = std::make_unique<fake_panel>();
  fake_panel* content_ptr = content.get();
  content_ptr->throw_on_draw = true;
  layer.add_panel(std::move(content));

  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  REQUIRE_THROWS_AS(context.ui->draw(), std::runtime_error);
  REQUIRE_FALSE(context.ui->frame_active());

  content_ptr->throw_on_draw = false;
  render(*context.ui);
  REQUIRE(content_ptr->draw_count == 2);
}

TEST_CASE("window names stay stable when the title changes", "[ui][panel]") {
  auto context = make_headless();
  auto& layer = add_layer(*context.ui, "app");
  auto content = std::make_unique<fake_panel>();
  fake_panel* content_ptr = content.get();
  content_ptr->title_value = "First";
  layer.add_panel(std::move(content));

  render(*context.ui);
  const std::string expected = window_name("app", "test.panel", "First");
  REQUIRE(ImGui::FindWindowByName(expected.c_str()) != nullptr);

  content_ptr->title_value = "Second";
  render(*context.ui);
  const std::string renamed = window_name("app", "test.panel", "Second");
  REQUIRE(ImGui::FindWindowByName(renamed.c_str()) != nullptr);
}
