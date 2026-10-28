/**
 * @file widgets_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Widget layout, sizing, and native content tests
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
using cyberverse::ui::rect;
using cyberverse::ui::system;
using cyberverse::ui::test::make_headless;
using cyberverse::ui::widgets::align_items;
using cyberverse::ui::widgets::flex_direction;
using cyberverse::ui::widgets::flexbox;
using cyberverse::ui::widgets::insets;
using cyberverse::ui::widgets::justify_content;
using cyberverse::ui::widgets::native;
using cyberverse::ui::widgets::text;
using cyberverse::ui::widgets::theme;
using cyberverse::ui::widgets::widget;

constexpr float width = 400.0f;
constexpr float height = 300.0f;
constexpr double frame_delta = 1.0 / 60.0;

class widget_panel final : public cyberverse::ui::panel {
public:
  explicit widget_panel(std::string id) : id_{ std::move(id) } {}

  [[nodiscard]] std::string_view id() const noexcept override { return id_; }
  [[nodiscard]] bool visible() const noexcept override { return true; }
  void draw(panel_context& context) override {
    ImGui::SetCursorScreenPos(ImGui::GetWindowPos());
    if (on_draw_) {
      on_draw_(context);
    }
  }

  std::function<void(panel_context&)> on_draw_;

private:
  std::string id_;
};

void run_frame(system& ui, widget_panel& panel) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

rect widget_rect(const widget& value) { return value.bounds(); }

} // namespace

TEST_CASE("flexbox lays out fixed, stretch, and gap", "[ui][widgets][layout]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));
  auto panel = std::make_unique<widget_panel>("widgets.root");
  widget_panel* panel_ptr = panel.get();

  rect root_rect{};
  rect first_rect{};
  rect second_rect{};

  panel_ptr->on_draw_ = [&](panel_context&) {
    flexbox root;
    root.size(300.0f, 100.0f).direction(flex_direction::row).gap(10.0f).padding(insets{ 5, 5, 5, 5 });
    auto& first = root.add(flexbox{}.width(50.0f).height_stretch());
    auto& second = root.add(flexbox{}.width_stretch().height(20.0f));
    cyberverse::ui::widgets::render(root);

    root_rect = widget_rect(root);
    first_rect = widget_rect(first);
    second_rect = widget_rect(second);
  };
  layer.add_panel(std::move(panel));

  run_frame(*context.ui, *panel_ptr);

  REQUIRE(root_rect.width == Catch::Approx(300.0f));
  REQUIRE(root_rect.height == Catch::Approx(100.0f));
  REQUIRE(first_rect.width == Catch::Approx(50.0f));
  REQUIRE(first_rect.height == Catch::Approx(90.0f));
  REQUIRE(first_rect.x == Catch::Approx(root_rect.x + 5.0f));
  REQUIRE(second_rect.width == Catch::Approx(230.0f));
  REQUIRE(second_rect.height == Catch::Approx(20.0f));
  REQUIRE(second_rect.x == Catch::Approx(first_rect.x + 50.0f + 10.0f));
}

TEST_CASE("margins offset children without changing their size", "[ui][widgets][layout]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));
  auto panel = std::make_unique<widget_panel>("widgets.margin");
  widget_panel* panel_ptr = panel.get();

  rect root_rect{};
  rect child_rect{};

  panel_ptr->on_draw_ = [&](panel_context&) {
    flexbox root;
    root.size(300.0f, 100.0f);
    auto& child = root.add(flexbox{}.size(50.0f, 20.0f).margin(insets{ 10.0f, 5.0f, 20.0f, 8.0f }));
    cyberverse::ui::widgets::render(root);
    root_rect = widget_rect(root);
    child_rect = widget_rect(child);
  };
  layer.add_panel(std::move(panel));

  run_frame(*context.ui, *panel_ptr);

  REQUIRE(child_rect.width == Catch::Approx(50.0f));
  REQUIRE(child_rect.height == Catch::Approx(20.0f));
  REQUIRE(child_rect.x == Catch::Approx(root_rect.x + 10.0f));
  REQUIRE(child_rect.y == Catch::Approx(root_rect.y + 5.0f));
}

TEST_CASE("fit-content text sizes itself", "[ui][widgets][text]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));
  auto panel = std::make_unique<widget_panel>("widgets.text");
  widget_panel* panel_ptr = panel.get();

  rect root_rect{};
  rect text_rect{};
  ImVec2 expected_text{};

  panel_ptr->on_draw_ = [&](panel_context&) {
    const ImVec2 text_size = ImGui::CalcTextSize("Hello widgets");
    expected_text = text_size;

    flexbox root;
    root.padding(insets{ 4, 6, 4, 6 });
    auto& label = root.add(text{ "Hello widgets" });
    cyberverse::ui::widgets::render(root);

    root_rect = widget_rect(root);
    text_rect = widget_rect(label);
  };
  layer.add_panel(std::move(panel));

  run_frame(*context.ui, *panel_ptr);

  REQUIRE(root_rect.width == Catch::Approx(expected_text.x + 8.0f));
  REQUIRE(root_rect.height == Catch::Approx(expected_text.y + 12.0f));
  REQUIRE(text_rect.width == Catch::Approx(expected_text.x));
  REQUIRE(text_rect.height == Catch::Approx(expected_text.y));
  REQUIRE(text_rect.x == Catch::Approx(root_rect.x + 4.0f));
  REQUIRE(text_rect.y == Catch::Approx(root_rect.y + 6.0f));
}

TEST_CASE("stretch fills the remaining column space", "[ui][widgets][layout]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));
  auto panel = std::make_unique<widget_panel>("widgets.stretch");
  widget_panel* panel_ptr = panel.get();

  rect first_rect{};
  rect second_rect{};

  panel_ptr->on_draw_ = [&](panel_context&) {
    flexbox root;
    root.size(200.0f, 100.0f).padding(insets{ 10, 10, 10, 10 }).gap(5.0f);
    auto& first = root.add(flexbox{}.height(20.0f).width_stretch());
    auto& second = root.add(flexbox{}.height_stretch().width_stretch());
    cyberverse::ui::widgets::render(root);

    first_rect = widget_rect(first);
    second_rect = widget_rect(second);
  };
  layer.add_panel(std::move(panel));

  run_frame(*context.ui, *panel_ptr);

  REQUIRE(first_rect.width == Catch::Approx(180.0f));
  REQUIRE(first_rect.height == Catch::Approx(20.0f));
  REQUIRE(second_rect.width == Catch::Approx(180.0f));
  REQUIRE(second_rect.height == Catch::Approx(55.0f));
  REQUIRE(second_rect.y == Catch::Approx(first_rect.y + 20.0f + 5.0f));
}

TEST_CASE("wrap measures a taller text block", "[ui][widgets][text]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));
  auto panel = std::make_unique<widget_panel>("widgets.wrap");
  widget_panel* panel_ptr = panel.get();

  float single_height = 0.0f;
  float wrapped_height = 0.0f;

  panel_ptr->on_draw_ = [&](panel_context&) {
    flexbox root;
    root.size(120.0f, 200.0f);
    auto& label = root.add(text{ "This is a long line that must wrap inside a narrow widget width." }.wrap());
    cyberverse::ui::widgets::render(root);
    wrapped_height = label.bounds().height;
    single_height = ImGui::CalcTextSize("This is a long line that must wrap inside a narrow widget width.").y;
  };
  layer.add_panel(std::move(panel));

  run_frame(*context.ui, *panel_ptr);

  REQUIRE(wrapped_height > single_height);
}

TEST_CASE("native content receives a measured rectangle", "[ui][widgets][native]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));
  auto panel = std::make_unique<widget_panel>("widgets.native");
  widget_panel* panel_ptr = panel.get();

  rect native_bounds{};
  bool called = false;

  panel_ptr->on_draw_ = [&](panel_context&) {
    flexbox root;
    root.size(200.0f, 100.0f)
        .direction(flex_direction::row)
        .justify(justify_content::center)
        .align(align_items::center);
    auto& content = root.add(native{ "plot",
                                     [&](const rect& bounds) {
                                       called = true;
                                       native_bounds = bounds;
                                       ImGui::GetWindowDrawList()->AddRectFilled(
                                           ImVec2{ bounds.x, bounds.y },
                                           ImVec2{ bounds.x + bounds.width, bounds.y + bounds.height },
                                           IM_COL32(255, 0, 0, 255));
                                     },
                                     [] {
                                       return cyberverse::ui::widgets::content_size{ 80.0f, 40.0f };
                                     } });
    cyberverse::ui::widgets::render(root);
  };
  layer.add_panel(std::move(panel));

  run_frame(*context.ui, *panel_ptr);

  REQUIRE(called);
  REQUIRE(native_bounds.width == Catch::Approx(80.0f));
  REQUIRE(native_bounds.height == Catch::Approx(40.0f));
}

TEST_CASE("theme overrides are accepted without changing layout", "[ui][widgets][theme]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets" }));
  auto panel = std::make_unique<widget_panel>("widgets.theme");
  widget_panel* panel_ptr = panel.get();

  float fixed_width = 0.0f;
  panel_ptr->on_draw_ = [&](panel_context&) {
    theme appearance;
    appearance.button.background = cyberverse::ui::widgets::color{ 0.1f, 0.2f, 0.3f, 1.0f };
    appearance.button.border =
        cyberverse::ui::widgets::border_style{ cyberverse::ui::widgets::color{ 1.0f, 1.0f, 1.0f, 1.0f }, 2.0f, 5.0f };

    flexbox root;
    root.size(200.0f, 100.0f);
    auto& control = root.add(cyberverse::ui::widgets::button{
        "OK", [] {
        } }.width(90.0f));
    cyberverse::ui::widgets::render(root, appearance);
    fixed_width = control.bounds().width;
  };
  layer.add_panel(std::move(panel));

  run_frame(*context.ui, *panel_ptr);
  REQUIRE(fixed_width == Catch::Approx(90.0f));
}
