/**
 * @file input_bridge_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Platform input to Dear ImGui bridge tests
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include "cyberverse/ui/host.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::platform::key_id;
using cyberverse::platform::mouse_button;
using cyberverse::ui::test::make_headless;

constexpr double frame_delta = 1.0 / 60.0;

} // namespace

TEST_CASE("mouse position, buttons, and wheel reach Dear ImGui", "[ui][input]") {
  auto context = make_headless(320, 200);
  context.ui->io().ConfigInputTrickleEventQueue = false;
  auto& window = context.headless_window();

  window.inject_mouse_move(12.0f, 34.0f);
  window.inject_mouse_button_down(mouse_button::left);
  window.inject_mouse_scroll(0.0f, 1.0f);
  window.poll_events();

  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, frame_delta));
  const ImGuiIO& io = ImGui::GetIO();
  REQUIRE(io.MousePos.x == 12.0f);
  REQUIRE(io.MousePos.y == 34.0f);
  REQUIRE(ImGui::IsMouseDown(ImGuiMouseButton_Left));
  REQUIRE(io.MouseWheel == 1.0f);
  context.ui->end_frame();
}

TEST_CASE("keys and text reach Dear ImGui", "[ui][input]") {
  auto context = make_headless(320, 200);
  context.ui->io().ConfigInputTrickleEventQueue = false;
  auto& window = context.headless_window();

  window.inject_key_down(key_id::key_w);
  window.inject_text("A");
  window.poll_events();

  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, frame_delta));
  REQUIRE(ImGui::IsKeyDown(ImGuiKey_W));
  const ImGuiIO& io = ImGui::GetIO();
  bool found = false;
  for (const ImWchar character : io.InputQueueCharacters) {
    if (character == static_cast<ImWchar>('A')) {
      found = true;
    }
  }
  REQUIRE(found);
  context.ui->end_frame();
}

TEST_CASE("focus loss releases keys held by the bridge", "[ui][input]") {
  auto context = make_headless(320, 200);
  context.ui->io().ConfigInputTrickleEventQueue = false;
  auto& window = context.headless_window();

  window.inject_key_down(key_id::key_w);
  window.poll_events();
  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, frame_delta));
  REQUIRE(ImGui::IsKeyDown(ImGuiKey_W));
  context.ui->end_frame();

  window.inject_focus_lost();
  window.poll_events();
  REQUIRE(context.ui->begin_frame(320.0f, 200.0f, frame_delta));
  REQUIRE_FALSE(ImGui::IsKeyDown(ImGuiKey_W));
  context.ui->end_frame();
}
