/**
 * @file radio_group_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets radio group tests
 * @version 0.1
 * @date 2026-10-31
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
#include "cyberverse/ui/host.hpp"
#include "cyberverse/ui/widgets.hpp"
#include "cyberverse/ui/window.hpp"
#include "cyberverse/ui/window_context.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::host;
using cyberverse::ui::window_context;
using cyberverse::ui::test::make_headless;

namespace widgets = cyberverse::ui::widgets;
using widgets::event_type;
using widgets::property_type;
using widgets::radio_group_options;
using widgets::radio_group_type;
using widgets::stack_direction;
using widgets::state;
using widgets::theme;

constexpr float width = 640.0f;
constexpr float height = 480.0f;
constexpr double frame_delta = 1.0 / 60.0;

class group_window final : public cyberverse::ui::window {
public:
  explicit group_window(std::function<void(window_context&)> draw)
      : window("test.radio_group"), draw_{ std::move(draw) } {}

  void draw(window_context& context) override {
    ImGui::SetCursorScreenPos(ImGui::GetWindowPos());
    draw_(context);
  }

private:
  std::function<void(window_context&)> draw_;
};

void run_frame(host& ui) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

void click(cyberverse::platform::backends::headless::window& window, host& ui, const cyberverse::ui::rect& target) {
  const auto left = cyberverse::platform::mouse_button::left;
  window.inject_mouse_move(target.x + target.width * 0.5f, target.y + target.height * 0.5f);
  window.inject_mouse_button_down(left);
  window.poll_events();
  run_frame(ui);
  window.inject_mouse_button_up(left);
  window.poll_events();
  run_frame(ui);
}

} // namespace

TEST_CASE("radio_group measures horizontal and vertical stack layouts", "[ui][widgets][radio_group]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  theme appearance;
  int horizontal_selected = -1;
  int vertical_selected = -1;

  state horizontal_s{ "radio.group.horizontal" };
  state vertical_s{ "radio.group.vertical" };

  auto window = std::make_unique<group_window>([&](window_context& window_context) {
    auto horizontal =
        widgets::radio_group(horizontal_s,
                             horizontal_selected,
                             radio_group_options{ .type = radio_group_type::button,
                                                  .layout = { .direction = stack_direction::horizontal, .gap = 8.0f } },
                             "First",
                             "Second");
    widgets::run(window_context, appearance, horizontal);

    auto vertical = widgets::radio_group(vertical_s,
                                         vertical_selected,
                                         radio_group_options{ .type = radio_group_type::radio },
                                         "First",
                                         "Second",
                                         "Third",
                                         "Fourth");
    widgets::run(window_context, appearance, vertical);
  });
  context.ui->add_window(std::move(window));
  run_frame(*context.ui);

  REQUIRE(horizontal_s.bounds().width > horizontal_s.bounds().height);
  REQUIRE(vertical_s.bounds().height > vertical_s.bounds().width);

  vertical_s.set(property_type::width, 300.0f);
  vertical_s.set(property_type::height, 80.0f);
  run_frame(*context.ui);
  REQUIRE(vertical_s.bounds().width == Catch::Approx(300.0f));
  REQUIRE(vertical_s.bounds().height == Catch::Approx(80.0f));
}

TEST_CASE("radio_group selects one radio and emits a change event", "[ui][widgets][radio_group]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  state group_s{ "radio.group.radio" };
  int selected = -1;
  int events = 0;
  int observed = -1;
  group_s.on_event([&](state&, event_type event) {
    if (event == event_type::value_changed) {
      ++events;
      observed = selected;
    }
  });

  auto window = std::make_unique<group_window>([&](window_context& window_context) {
    auto group = widgets::radio_group(group_s, selected, radio_group_options{}, "Only");
    widgets::run(window_context, widgets::theme{}, group);
  });
  context.ui->add_window(std::move(window));
  run_frame(*context.ui);

  const cyberverse::ui::rect bounds = group_s.bounds();
  click(context.headless_window(), *context.ui, bounds);
  REQUIRE(selected == 0);
  REQUIRE(events == 1);
  REQUIRE(observed == 0);

  click(context.headless_window(), *context.ui, bounds);
  REQUIRE(selected == 0);
  REQUIRE(events == 1);
}

TEST_CASE("radio_group button type selects and highlights its option", "[ui][widgets][radio_group]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  state group_s{ "radio.group.button" };
  int selected = -1;
  int events = 0;
  group_s.on_event([&](state&, event_type event) {
    if (event == event_type::value_changed) {
      ++events;
    }
  });

  auto window = std::make_unique<group_window>([&](window_context& window_context) {
    auto group = widgets::radio_group(
        group_s,
        selected,
        radio_group_options{ .type = radio_group_type::button, .layout = { .direction = stack_direction::horizontal } },
        "One",
        "Two");
    widgets::run(window_context, widgets::theme{}, group);
  });
  context.ui->add_window(std::move(window));
  run_frame(*context.ui);

  const cyberverse::ui::rect bounds = group_s.bounds();
  const cyberverse::ui::rect second{ bounds.x + bounds.width - 4.0f, bounds.y, 4.0f, bounds.height };
  click(context.headless_window(), *context.ui, second);
  REQUIRE(selected == 1);
  REQUIRE(events == 1);
}

TEST_CASE("radio_group repairs an out-of-range selection", "[ui][widgets][radio_group]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  state group_s{ "radio.group.invalid" };
  int selected = 99;
  int events = 0;
  group_s.on_event([&](state&, event_type event) {
    if (event == event_type::value_changed) {
      ++events;
    }
  });

  auto window = std::make_unique<group_window>([&](window_context& window_context) {
    auto group = widgets::radio_group(group_s, selected, radio_group_options{}, "Only");
    widgets::run(window_context, widgets::theme{}, group);
  });
  context.ui->add_window(std::move(window));
  run_frame(*context.ui);

  const cyberverse::ui::rect bounds = group_s.bounds();
  click(context.headless_window(), *context.ui, bounds);
  REQUIRE(selected == 0);
  REQUIRE(events == 1);
}

TEST_CASE("radio_group zero width skips drawing", "[ui][widgets][radio_group][zero]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  state group_s{ "radio.group.zero" };
  int selected = -1;
  int events = 0;
  group_s.on_event([&](state&, event_type event) {
    if (event == event_type::value_changed) {
      ++events;
    }
  });
  group_s.set(property_type::width, 0.0f);

  auto window = std::make_unique<group_window>([&](window_context& window_context) {
    auto group = widgets::radio_group(group_s, selected, radio_group_options{}, "Only");
    widgets::run(window_context, widgets::theme{}, group);
  });
  context.ui->add_window(std::move(window));
  run_frame(*context.ui);

  REQUIRE(group_s.bounds().width == Catch::Approx(0.0f));
  REQUIRE(selected == -1);
  REQUIRE(events == 0);
}
