/**
 * @file window_ordering_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Window ordering, visibility, and ownership tests
 * @version 0.4
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <stdexcept>
#include <string>
#include <vector>

#include "cyberverse/ui/host.hpp"
#include "cyberverse/ui/window.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::host;
using cyberverse::ui::window_context;
using cyberverse::ui::test::fake_window;
using cyberverse::ui::test::make_headless;

constexpr float width = 320.0f;
constexpr float height = 200.0f;
constexpr double frame_delta = 1.0 / 60.0;

fake_window& add_recording_window(host& ui, std::string id, int order, std::vector<std::string>& draw_order) {
  auto content = std::make_unique<fake_window>(std::move(id));
  content->order_value = order;
  const std::string captured_id = std::string{ content->id() };
  content->on_draw = [&draw_order, captured_id](window_context&) {
    draw_order.push_back(captured_id);
  };
  return static_cast<fake_window&>(ui.add_window(std::move(content)));
}

void render(host& ui) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

} // namespace

TEST_CASE("windows draw from the lowest order and keep insertion order on ties", "[ui][window][order]") {
  auto context = make_headless();
  std::vector<std::string> draw_order;

  add_recording_window(*context.ui, "high.window", 20, draw_order);
  add_recording_window(*context.ui, "first.window", 10, draw_order);
  add_recording_window(*context.ui, "second.window", 10, draw_order);

  render(*context.ui);
  REQUIRE(draw_order == std::vector<std::string>{ "first.window", "second.window", "high.window" });
}

TEST_CASE("window order changes rebuild the cached draw order", "[ui][window][order]") {
  auto context = make_headless();
  std::vector<std::string> draw_order;

  fake_window& a = add_recording_window(*context.ui, "a.window", 0, draw_order);
  add_recording_window(*context.ui, "b.window", 10, draw_order);

  render(*context.ui);
  REQUIRE(draw_order == std::vector<std::string>{ "a.window", "b.window" });

  draw_order.clear();
  a.order_value = 30;
  render(*context.ui);
  REQUIRE(draw_order == std::vector<std::string>{ "b.window", "a.window" });
}

TEST_CASE("hidden windows are skipped", "[ui][window][visibility]") {
  auto context = make_headless();
  std::vector<std::string> draw_order;

  fake_window& content = add_recording_window(*context.ui, "hidden.window", 0, draw_order);
  content.visible_value = false;

  REQUIRE_FALSE(context.ui->has_visible_content());
  render(*context.ui);
  REQUIRE(draw_order.empty());
  REQUIRE(content.draw_count == 0);

  content.visible_value = true;
  REQUIRE(context.ui->has_visible_content());
  render(*context.ui);
  REQUIRE(draw_order == std::vector<std::string>{ "hidden.window" });
}

TEST_CASE("window ids are validated in the constructor and unique in the host", "[ui][window]") {
  REQUIRE_THROWS_AS(fake_window{ "" }, std::invalid_argument);
  REQUIRE_THROWS_AS(fake_window{ "bad###id" }, std::invalid_argument);
  REQUIRE_THROWS_AS((fake_window{ std::string{ "bad\0id", 6 } }), std::invalid_argument);

  auto context = make_headless();
  auto first = std::make_unique<fake_window>("same");
  context.ui->add_window(std::move(first));

  auto duplicate = std::make_unique<fake_window>("same");
  REQUIRE_THROWS_AS(context.ui->add_window(std::move(duplicate)), std::invalid_argument);
}

TEST_CASE("windows are owned, findable, and removable", "[ui][window]") {
  auto context = make_headless();
  auto content = std::make_unique<fake_window>("owned.window");
  fake_window* content_ptr = content.get();
  context.ui->add_window(std::move(content));

  REQUIRE(context.ui->has_visible_content());
  REQUIRE(context.ui->find_window("owned.window") == content_ptr);
  REQUIRE(context.ui->remove_window(*content_ptr));
  REQUIRE_FALSE(context.ui->remove_window(*content_ptr));
  REQUIRE_FALSE(context.ui->has_visible_content());
  REQUIRE(context.ui->find_window("owned.window") == nullptr);

  context.ui->add_window(std::make_unique<fake_window>());
  context.ui->clear_windows();
  REQUIRE_FALSE(context.ui->has_visible_content());
}

TEST_CASE("window mutations during a frame are rejected", "[ui][window]") {
  auto context = make_headless();
  auto content = std::make_unique<fake_window>("frame.window");
  fake_window* content_ptr = content.get();
  context.ui->add_window(std::move(content));

  REQUIRE(context.ui->begin_frame(width, height, frame_delta));
  REQUIRE_FALSE(context.ui->remove_window(*content_ptr));
  REQUIRE_THROWS_AS(context.ui->clear_windows(), std::logic_error);
  REQUIRE_THROWS_AS(context.ui->add_window(std::make_unique<fake_window>()), std::logic_error);
  context.ui->end_frame();
}
