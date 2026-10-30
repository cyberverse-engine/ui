/**
 * @file frame_stage_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief ui::frame_stage runtime adapter tests without a GPU
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

#include "cyberverse/render_core/frame_context.hpp"
#include "cyberverse/runtime/frame_budget.hpp"
#include "cyberverse/ui/frame_stage.hpp"
#include "cyberverse/ui/layer.hpp"
#include "cyberverse/ui/system.hpp"
#include "cyberverse/ui/window.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::render_core::frame_context;
using cyberverse::render_core::viewport;
using cyberverse::runtime::frame_budget;
using cyberverse::ui::frame_stage;
using cyberverse::ui::layer;
using cyberverse::ui::layer_desc;
using cyberverse::ui::system;
using cyberverse::ui::test::fake_window;
using cyberverse::ui::test::make_headless;

constexpr double frame_delta = 1.0 / 60.0;

frame_context detached_frame() {
  frame_context frame;
  viewport vp;
  vp.width = 320.0f;
  vp.height = 200.0f;
  frame.set_viewport(vp);
  return frame;
}

} // namespace

TEST_CASE("the runtime adapter drives one ImGui frame through its provider", "[ui][runtime-stage]") {
  auto context = make_headless(320, 200, false);
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "app" }));
  auto content = std::make_unique<fake_window>();
  fake_window* content_ptr = content.get();
  layer.add_window(std::move(content));

  int provider_calls = 0;
  frame_stage stage{ *context.ui, [&provider_calls](double, double, const frame_budget&) {
                      ++provider_calls;
                      return detached_frame();
                    } };

  const int frame_count_before = ImGui::GetFrameCount();
  stage.on_render(frame_delta, 1.0, frame_budget::unlimited());

  REQUIRE(provider_calls == 1);
  REQUIRE(content_ptr->draw_count == 1);
  REQUIRE(ImGui::GetFrameCount() == frame_count_before + 1);
  REQUIRE_FALSE(context.ui->frame_active());
}

TEST_CASE("the runtime adapter skips the provider without visible content", "[ui][runtime-stage]") {
  auto context = make_headless(320, 200, false);
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "app" }));
  auto content = std::make_unique<fake_window>();
  content->visible_value = false;
  layer.add_window(std::move(content));

  int provider_calls = 0;
  frame_stage stage{ *context.ui, [&provider_calls](double, double, const frame_budget&) {
                      ++provider_calls;
                      return detached_frame();
                    } };

  const int frame_count_before = ImGui::GetFrameCount();
  stage.on_render(frame_delta, 1.0, frame_budget::unlimited());

  REQUIRE(provider_calls == 0);
  REQUIRE(ImGui::GetFrameCount() == frame_count_before);
}

TEST_CASE("the runtime adapter ignores an invalid frame", "[ui][runtime-stage]") {
  auto context = make_headless(320, 200, false);
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "app" }));
  auto content = std::make_unique<fake_window>();
  fake_window* content_ptr = content.get();
  layer.add_window(std::move(content));

  frame_stage stage{ *context.ui, [](double, double, const frame_budget&) {
                      return frame_context{};
                    } };
  stage.on_render(frame_delta, 1.0, frame_budget::unlimited());

  REQUIRE(content_ptr->draw_count == 0);
  REQUIRE_FALSE(context.ui->frame_active());
}

TEST_CASE("window exceptions propagate through the runtime adapter", "[ui][runtime-stage]") {
  auto context = make_headless(320, 200, false);
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "app" }));
  auto content = std::make_unique<fake_window>();
  content->throw_on_draw = true;
  layer.add_window(std::move(content));

  frame_stage stage{ *context.ui, [](double, double, const frame_budget&) {
                      return detached_frame();
                    } };
  REQUIRE_THROWS_AS(stage.on_render(frame_delta, 1.0, frame_budget::unlimited()), std::runtime_error);
  REQUIRE_FALSE(context.ui->frame_active());
}

TEST_CASE("an empty provider is rejected", "[ui][runtime-stage]") {
  auto context = make_headless();
  REQUIRE_THROWS_AS(frame_stage(*context.ui, cyberverse::ui::frame_provider{}), std::invalid_argument);
}
