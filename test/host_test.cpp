/**
 * @file host_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief UI host lifecycle tests
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <limits>
#include <memory>
#include <stdexcept>

#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::host;
using cyberverse::ui::host_config;
using cyberverse::ui::window_context;
using cyberverse::ui::test::fake_window;
using cyberverse::ui::test::make_headless;

} // namespace

TEST_CASE("headless host pairs begin and end", "[ui][host]") {
  auto context = make_headless();
  REQUIRE_FALSE(context.ui->frame_active());

  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, 1.0 / 60.0));
  REQUIRE(context.ui->frame_active());
  REQUIRE_FALSE(context.ui->begin_frame(320.0f, 200.0f, 1.0 / 60.0));
  REQUIRE(context.ui->frame_active());

  context.ui->draw();
  context.ui->end_frame();
  REQUIRE_FALSE(context.ui->frame_active());

  context.ui->end_frame();
  REQUIRE_FALSE(context.ui->frame_active());
}

TEST_CASE("headless begin_frame rejects unusable sizes", "[ui][host]") {
  auto context = make_headless();

  REQUIRE_FALSE(context.ui->begin_frame(0.0f, 100.0f, 1.0 / 60.0));
  REQUIRE_FALSE(context.ui->begin_frame(100.0f, 0.0f, 1.0 / 60.0));
  REQUIRE_FALSE(context.ui->begin_frame(-10.0f, 100.0f, 1.0 / 60.0));
  REQUIRE_FALSE(context.ui->begin_frame(std::numeric_limits<float>::quiet_NaN(), 100.0f, 1.0 / 60.0));
  REQUIRE_FALSE(context.ui->frame_active());
}

TEST_CASE("a non-headless host requires a device and window", "[ui][host]") {
  host_config config;
  REQUIRE_THROWS_AS(host::create(config), std::invalid_argument);
}

TEST_CASE("headless frame produces draw data exactly once", "[ui][host]") {
  auto context = make_headless();
  auto content = std::make_unique<fake_window>();
  content->on_draw = [](window_context&) {
    ImGui::TextUnformatted("cyberverse");
  };
  context.ui->add_window(std::move(content));

  const int frame_count_before = ImGui::GetFrameCount();
  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, 1.0 / 60.0));
  context.ui->draw();
  context.ui->end_frame();

  REQUIRE(ImGui::GetFrameCount() == frame_count_before + 1);
  const ImDrawData* draw_data = ImGui::GetDrawData();
  REQUIRE(draw_data != nullptr);
  REQUIRE(draw_data->CmdListsCount > 0);
  REQUIRE(draw_data->TotalVtxCount > 0);
}

TEST_CASE("destroying a host with an active frame is safe", "[ui][host]") {
  {
    auto context = make_headless();
    REQUIRE(context.ui->begin_frame(320.0f, 200.0f, 1.0 / 60.0));
    REQUIRE(context.ui->frame_active());
  }
  SUCCEED();
}

TEST_CASE("structural mutations during a frame are rejected", "[ui][host]") {
  auto context = make_headless();
  auto content = std::make_unique<fake_window>();
  fake_window* content_ptr = content.get();
  context.ui->add_window(std::move(content));

  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, 1.0 / 60.0));
  REQUIRE_THROWS_AS(context.ui->add_window(std::make_unique<fake_window>()), std::logic_error);
  REQUIRE_FALSE(context.ui->remove_window(*content_ptr));
  REQUIRE_THROWS_AS(context.ui->clear_windows(), std::logic_error);
  context.ui->end_frame();
}

TEST_CASE("capture flags default to false and font ids are reported", "[ui][host]") {
  auto context = make_headless();
  REQUIRE_FALSE(context.ui->wants_mouse());
  REQUIRE_FALSE(context.ui->wants_keyboard());
  REQUIRE(context.ui->font_loaded("default"));
  REQUIRE_FALSE(context.ui->font_loaded("cjk"));
  REQUIRE(context.ui->find_window("missing") == nullptr);
  REQUIRE_FALSE(context.ui->has_visible_content());
}
