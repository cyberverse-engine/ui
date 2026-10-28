/**
 * @file widgets_containers_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Scroll view and collapse group tests
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <memory>
#include <string>

#include <imgui.h>

#include "cyberverse/platform/backends/headless/window.hpp"
#include "cyberverse/platform/raw_input.hpp"
#include "cyberverse/ui/layer.hpp"
#include "cyberverse/ui/panel.hpp"
#include "cyberverse/ui/panel_context.hpp"
#include "cyberverse/ui/system.hpp"
#include "cyberverse/ui/widgets.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::layer;
using cyberverse::ui::layer_desc;
using cyberverse::ui::panel_context;
using cyberverse::ui::system;
using cyberverse::ui::test::make_headless;
using cyberverse::ui::widgets::collapse_group;
using cyberverse::ui::widgets::flexbox;
using cyberverse::ui::widgets::scroll_view;
using cyberverse::ui::widgets::text;

constexpr float width = 320.0f;
constexpr float height = 240.0f;
constexpr double frame_delta = 1.0 / 60.0;

class widget_panel final : public cyberverse::ui::panel {
public:
  explicit widget_panel(std::string id, std::function<void()> on_draw)
      : id_{ std::move(id) }, on_draw_{ std::move(on_draw) } {}

  [[nodiscard]] std::string_view id() const noexcept override { return id_; }
  [[nodiscard]] bool visible() const noexcept override { return true; }

  void draw(panel_context& /*context*/) override {
    ImGui::SetCursorScreenPos(ImGui::GetWindowPos());
    on_draw_();
  }

private:
  std::string id_;
  std::function<void()> on_draw_;
};

void run_frame(system& ui) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

} // namespace

TEST_CASE("scroll view lays out content inside a native child", "[ui][widgets][scroll]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));

  cyberverse::ui::rect view_bounds{};
  cyberverse::ui::rect content_bounds{};
  auto panel = std::make_unique<widget_panel>("widgets.scroll", [&] {
    flexbox root;
    root.size(240.0f, 160.0f);
    auto& view = root.add(scroll_view{}.height(100.0f).width(200.0f).gap(4.0f));
    for (int index = 0; index < 8; ++index) {
      view.add(text{ "row " + std::to_string(index) });
    }
    cyberverse::ui::widgets::render(root);
    view_bounds = view.bounds();
    content_bounds = view.content().bounds();
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);

  REQUIRE(view_bounds.width == Catch::Approx(200.0f));
  REQUIRE(view_bounds.height == Catch::Approx(100.0f));
  REQUIRE(content_bounds.height > view_bounds.height);
}

TEST_CASE("collapse group measures its body only while open", "[ui][widgets][collapse]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));

  bool open = false;
  cyberverse::ui::rect closed_bounds{};
  cyberverse::ui::rect open_bounds{};
  bool first_frame = true;

  auto panel = std::make_unique<widget_panel>("widgets.collapse", [&] {
    flexbox root;
    root.size(240.0f, 200.0f);
    auto& group = root.add(collapse_group{ "Group", open }.gap(6.0f));
    group.add(text{ "body line 1" });
    group.add(text{ "body line 2" });
    cyberverse::ui::widgets::render(root);
    if (first_frame) {
      closed_bounds = group.bounds();
      first_frame = false;
    } else {
      open_bounds = group.bounds();
    }
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  REQUIRE_FALSE(open);

  auto& window = context.headless_window();
  const auto left = cyberverse::platform::mouse_button::left;
  window.inject_mouse_move(closed_bounds.x + 20.0f, closed_bounds.y + closed_bounds.height * 0.5f);
  window.inject_mouse_button_down(left);
  window.poll_events();
  run_frame(*context.ui);
  window.inject_mouse_button_up(left);
  window.poll_events();
  run_frame(*context.ui);
  run_frame(*context.ui);

  REQUIRE(open);
  REQUIRE(open_bounds.height > closed_bounds.height);
}
