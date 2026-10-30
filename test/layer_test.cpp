/**
 * @file layer_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief UI layer ordering, visibility, and ownership tests
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

#include "cyberverse/ui/layer.hpp"
#include "cyberverse/ui/system.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::layer;
using cyberverse::ui::layer_desc;
using cyberverse::ui::system;
using cyberverse::ui::window_context;
using cyberverse::ui::test::fake_window;
using cyberverse::ui::test::make_headless;

constexpr float width = 320.0f;
constexpr float height = 200.0f;
constexpr double frame_delta = 1.0 / 60.0;

layer& add_layer(system& ui, std::string name, int order) {
  return ui.add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = std::move(name), .order = order }));
}

fake_window& add_recording_window(layer& layer, std::string id, std::vector<std::string>& draw_order) {
  auto content = std::make_unique<fake_window>();
  content->id_value = std::move(id);
  content->on_draw = [&draw_order, id = content->id_value](window_context&) {
    draw_order.push_back(id);
  };
  return static_cast<fake_window&>(layer.add_window(std::move(content)));
}

void render(system& ui) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

} // namespace

TEST_CASE("layers draw from the lowest order and keep insertion order on ties", "[ui][layer]") {
  auto context = make_headless();
  std::vector<std::string> draw_order;

  auto& high = add_layer(*context.ui, "high", 20);
  add_recording_window(high, "high.window", draw_order);
  auto& first = add_layer(*context.ui, "first", 10);
  add_recording_window(first, "first.window", draw_order);
  auto& second = add_layer(*context.ui, "second", 10);
  add_recording_window(second, "second.window", draw_order);

  render(*context.ui);

  REQUIRE(draw_order == std::vector<std::string>{ "first.window", "second.window", "high.window" });
}

TEST_CASE("order changes rebuild the cached draw order", "[ui][layer]") {
  auto context = make_headless();
  std::vector<std::string> draw_order;

  auto& a = add_layer(*context.ui, "a", 0);
  add_recording_window(a, "a.window", draw_order);
  auto& b = add_layer(*context.ui, "b", 10);
  add_recording_window(b, "b.window", draw_order);

  render(*context.ui);
  REQUIRE(draw_order == std::vector<std::string>{ "a.window", "b.window" });

  draw_order.clear();
  a.set_order(30);
  render(*context.ui);
  REQUIRE(draw_order == std::vector<std::string>{ "b.window", "a.window" });
}

TEST_CASE("hidden layers and hidden windows are skipped", "[ui][layer]") {
  auto context = make_headless();
  std::vector<std::string> draw_order;

  auto& hidden_layer = add_layer(*context.ui, "hidden", 0);
  auto& content = add_recording_window(hidden_layer, "hidden.window", draw_order);
  hidden_layer.set_visible(false);

  REQUIRE_FALSE(context.ui->has_visible_content());
  render(*context.ui);
  REQUIRE(draw_order.empty());
  REQUIRE(content.draw_count == 0);

  hidden_layer.set_visible(true);
  content.visible_value = false;
  REQUIRE_FALSE(context.ui->has_visible_content());
  render(*context.ui);
  REQUIRE(draw_order.empty());
}

TEST_CASE("layer names and window ids are validated", "[ui][layer]") {
  REQUIRE_THROWS_AS(layer{ layer_desc{ .name = "" } }, std::invalid_argument);
  REQUIRE_THROWS_AS(layer{ layer_desc{ .name = "bad###name" } }, std::invalid_argument);
  REQUIRE_THROWS_AS(layer{ layer_desc{ .name = "bad/name" } }, std::invalid_argument);

  auto context = make_headless();
  auto& layer = add_layer(*context.ui, "app", 0);
  REQUIRE_THROWS_AS(add_layer(*context.ui, "app", 0), std::invalid_argument);

  auto first = std::make_unique<fake_window>();
  first->id_value = "same";
  layer.add_window(std::move(first));

  auto duplicate = std::make_unique<fake_window>();
  duplicate->id_value = "same";
  REQUIRE_THROWS_AS(layer.add_window(std::move(duplicate)), std::invalid_argument);

  auto empty = std::make_unique<fake_window>();
  empty->id_value = "";
  REQUIRE_THROWS_AS(layer.add_window(std::move(empty)), std::invalid_argument);

  auto separator = std::make_unique<fake_window>();
  separator->id_value = "bad/id";
  REQUIRE_THROWS_AS(layer.add_window(std::move(separator)), std::invalid_argument);
}

TEST_CASE("windows and layers are owned and removable", "[ui][layer]") {
  auto context = make_headless();
  auto& layer = add_layer(*context.ui, "app", 0);
  auto content = std::make_unique<fake_window>();
  fake_window* content_ptr = content.get();
  layer.add_window(std::move(content));

  REQUIRE(context.ui->has_visible_content());
  REQUIRE(context.ui->find_layer("app") == &layer);
  REQUIRE(layer.remove_window(*content_ptr));
  REQUIRE_FALSE(layer.remove_window(*content_ptr));
  REQUIRE_FALSE(context.ui->has_visible_content());

  REQUIRE(context.ui->remove_layer(layer));
  REQUIRE_FALSE(context.ui->remove_layer(layer));
  REQUIRE(context.ui->find_layer("app") == nullptr);
}

TEST_CASE("layer mutations during a frame are rejected", "[ui][layer]") {
  auto context = make_headless();
  auto& layer = add_layer(*context.ui, "app", 0);
  auto content = std::make_unique<fake_window>();
  fake_window* content_ptr = content.get();
  layer.add_window(std::move(content));

  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  REQUIRE_THROWS_AS(layer.clear_windows(), std::logic_error);
  REQUIRE_FALSE(layer.remove_window(*content_ptr));
  REQUIRE_THROWS_AS(context.ui->clear_layers(), std::logic_error);
  context.ui->end_frame();
}
