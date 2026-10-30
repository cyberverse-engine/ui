/**
 * @file z_order_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Layer z-order protection tests
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>

#include "cyberverse/ui/detail/z_order.hpp"
#include "cyberverse/ui/layer.hpp"
#include "cyberverse/ui/system.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::layer;
using cyberverse::ui::layer_desc;
using cyberverse::ui::system;
using cyberverse::ui::detail::display_order;
using cyberverse::ui::test::fake_window;
using cyberverse::ui::test::make_headless;
using cyberverse::ui::test::window_name;

constexpr double frame_delta = 1.0 / 60.0;

layer& add_layer(system& ui, std::string name, int order, const std::string& window_id) {
  auto layer = std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = std::move(name), .order = order });
  auto content = std::make_unique<fake_window>();
  content->id_value = window_id;
  layer->add_window(std::move(content));
  return ui.add_layer(std::move(layer));
}

} // namespace

TEST_CASE("higher-order layers stay above lower layers", "[ui][z-order]") {
  auto context = make_headless(320, 200);
  add_layer(*context.ui, "bottom", 0, "bottom.window");
  add_layer(*context.ui, "top", 10, "top.window");

  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, frame_delta));
  context.ui->draw();

  const std::string bottom_name = window_name("bottom", "bottom.window");
  const std::string top_name = window_name("top", "top.window");
  REQUIRE(display_order(top_name.c_str(), bottom_name.c_str()) == 1);
  REQUIRE(display_order(bottom_name.c_str(), top_name.c_str()) == -1);

  context.ui->end_frame();
}

TEST_CASE("relative order is stable for equal-order layers", "[ui][z-order]") {
  auto context = make_headless(320, 200);
  add_layer(*context.ui, "first", 5, "first.window");
  add_layer(*context.ui, "second", 5, "second.window");

  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, frame_delta));
  context.ui->draw();

  const std::string first_name = window_name("first", "first.window");
  const std::string second_name = window_name("second", "second.window");
  REQUIRE(display_order(second_name.c_str(), first_name.c_str()) == 1);

  context.ui->end_frame();
}
