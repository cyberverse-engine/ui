/**
 * @file z_order_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Window z-order protection tests
 * @version 0.4
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <memory>
#include <string>

#include "cyberverse/ui/detail/z_order.hpp"
#include "cyberverse/ui/host.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::host;
using cyberverse::ui::detail::display_order;
using cyberverse::ui::test::fake_window;
using cyberverse::ui::test::make_headless;

constexpr double frame_delta = 1.0 / 60.0;

void add_window(host& ui, std::string id, int order) {
  auto content = std::make_unique<fake_window>(std::move(id));
  content->order_value = order;
  ui.add_window(std::move(content));
}

} // namespace

TEST_CASE("higher-order windows stay above lower windows", "[ui][z-order]") {
  auto context = make_headless(320, 200);
  add_window(*context.ui, "bottom.window", 0);
  add_window(*context.ui, "top.window", 10);

  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, frame_delta));
  context.ui->draw();

  REQUIRE(display_order("top.window", "bottom.window") == 1);
  REQUIRE(display_order("bottom.window", "top.window") == -1);

  context.ui->end_frame();
}

TEST_CASE("relative order is stable for equal-order windows", "[ui][z-order]") {
  auto context = make_headless(320, 200);
  add_window(*context.ui, "first.window", 5);
  add_window(*context.ui, "second.window", 5);

  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, frame_delta));
  context.ui->draw();

  REQUIRE(display_order("second.window", "first.window") == 1);

  context.ui->end_frame();
}
