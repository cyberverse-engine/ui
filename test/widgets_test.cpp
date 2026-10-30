/**
 * @file widgets_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets state, component, stack, and button tests
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_approx.hpp>
#include <catch2/catch_test_macros.hpp>

#include <functional>
#include <memory>
#include <string>
#include <type_traits>

#include <imgui.h>

#include "cyberverse/platform/backends/headless/window.hpp"
#include "cyberverse/platform/raw_input.hpp"
#include "cyberverse/ui/host.hpp"
#include "cyberverse/ui/widgets.hpp"
#include "cyberverse/ui/widgets/detail/state_access.hpp"
#include "cyberverse/ui/window.hpp"
#include "cyberverse/ui/window_context.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::host;
using cyberverse::ui::window_context;
using cyberverse::ui::test::make_headless;

namespace widgets = cyberverse::ui::widgets;
using widgets::align_items;
using widgets::animator;
using widgets::button_style;
using widgets::divider_options;
using widgets::divider_orientation;
using widgets::event_type;
using widgets::flex_direction;
using widgets::flex_options;
using widgets::frame_context;
using widgets::insets;
using widgets::justify_content;
using widgets::property_type;
using widgets::rect;
using widgets::scroll_content_direction;
using widgets::scroll_options;
using widgets::stack_direction;
using widgets::stack_options;
using widgets::state;
using widgets::theme;
using widgets::update_context;
using widgets::visibility;

struct probe_impl {
  float* observed_opacity = nullptr;
  int* draw_count = nullptr;
  int* measure_count = nullptr;

  [[nodiscard]] widgets::layout_size
  measure(const widgets::state&, const widgets::theme&, const widgets::constraints&) const noexcept {
    if (measure_count != nullptr) {
      ++*measure_count;
    }
    return widgets::layout_size{ 10.0f, 10.0f };
  }

  void draw(widgets::state&, widgets::draw_context& ctx) {
    if (observed_opacity != nullptr) {
      *observed_opacity = ctx.opacity;
    }
    if (draw_count != nullptr) {
      ++*draw_count;
    }
  }

  template <typename F>
  void visit_states(F&& /*visitor*/) const noexcept {}
};

constexpr float width = 640.0f;
constexpr float height = 480.0f;
constexpr float frame_delta = 1.0f / 60.0f;

class host_window final : public cyberverse::ui::window {
public:
  explicit host_window(std::string id, std::function<void(window_context&)> draw)
      : window(std::move(id)), draw_{ std::move(draw) } {}

  [[nodiscard]] bool visible() const noexcept override { return true; }

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

void click(cyberverse::platform::backends::headless::window& window, host& ui, const rect& target) {
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

TEST_CASE("widgets state stores immediate and animated values", "[ui][widgets][state]") {
  state value{ "widgets.state" };
  animator animations;

  REQUIRE(value.value(property_type::opacity) == Catch::Approx(1.0f));
  value.set(property_type::height, 100.0f);
  REQUIRE(value.value(property_type::height) == Catch::Approx(100.0f));

  value.animate(
      property_type::height, 0.0f, { .duration = 1.0f, .easing = cyberverse::animation::easing_type::linear });
  update_context update{ 0.5f, animations };
  value.tick(update);
  REQUIRE(value.value(property_type::height) == Catch::Approx(50.0f));

  value.tick(update);
  REQUIRE(value.value(property_type::height) == Catch::Approx(0.0f));
  REQUIRE_FALSE(value.has(property_type::width));
}

TEST_CASE("widgets state is non-copyable and non-movable", "[ui][widgets][state]") {
  STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<state>);
  STATIC_REQUIRE_FALSE(std::is_move_constructible_v<state>);
}

TEST_CASE("widgets component applies visibility and opacity centrally", "[ui][widgets][component]") {
  state value{ "probe" };
  value.set(property_type::opacity, 0.25f);
  theme appearance;
  int draws = 0;
  float observed_opacity = 0.0f;
  widgets::component<probe_impl> probe{ value, probe_impl{ &observed_opacity, &draws } };
  widgets::detail::state_access::set_bounds(value, rect{ 0.0f, 0.0f, 10.0f, 10.0f });

  widgets::draw_context context{ appearance, 1.0f };
  probe.draw(context);
  REQUIRE(draws == 1);
  REQUIRE(observed_opacity == Catch::Approx(0.25f));

  widgets::detail::state_access::set_bounds(value, rect{});
  probe.draw(context);
  REQUIRE(draws == 1);
  widgets::detail::state_access::set_bounds(value, rect{ 0.0f, 0.0f, 10.0f, 10.0f });

  value.set_visibility(visibility::invisible);
  probe.draw(context);
  REQUIRE(draws == 1);

  value.set_visibility(visibility::collapsed);
  const widgets::layout_size collapsed = probe.measure(appearance, widgets::constraints{});
  REQUIRE(collapsed.width == Catch::Approx(0.0f));
  REQUIRE(collapsed.height == Catch::Approx(0.0f));
}

TEST_CASE("widgets box clips and pads its child", "[ui][widgets][box]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state box_s{ "box" };
  state child_s{ "box.child" };
  box_s.set(property_type::width, 100.0f);
  box_s.set(property_type::height, 80.0f);
  child_s.set(property_type::width, 40.0f);
  child_s.set(property_type::height, 20.0f);
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.box", [&](window_context&) {
    auto child = widgets::text(child_s, "boxed");
    auto root = widgets::box(box_s, widgets::box_options{ .padding = insets{ 5.0f, 6.0f, 7.0f, 8.0f } }, child);
    widgets::run(root, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window));
  run_frame(*context.ui);

  REQUIRE(box_s.bounds().width == Catch::Approx(100.0f));
  REQUIRE(box_s.bounds().height == Catch::Approx(80.0f));
  REQUIRE(child_s.bounds().x == Catch::Approx(box_s.bounds().x + 5.0f));
  REQUIRE(child_s.bounds().y == Catch::Approx(box_s.bounds().y + 6.0f));
  REQUIRE(child_s.bounds().width == Catch::Approx(100.0f - 5.0f - 7.0f));
  REQUIRE(child_s.bounds().height == Catch::Approx(80.0f - 6.0f - 8.0f));
}

TEST_CASE("widgets zero-size containers skip child computation", "[ui][widgets][layout][zero]") {
  state container_s{ "zero.container" };
  state child_s{ "zero.child" };
  container_s.set(property_type::width, 0.0f);

  int measures = 0;
  int draws = 0;
  auto child = widgets::component<probe_impl>{ child_s, probe_impl{ nullptr, &draws, &measures } };
  auto root = widgets::stack(container_s, stack_options{}, child);

  theme appearance;
  const widgets::layout_size measured = root.measure(appearance, widgets::constraints{ 640.0f, 480.0f });
  REQUIRE(measured.width == Catch::Approx(0.0f));
  REQUIRE(measured.height == Catch::Approx(0.0f));
  REQUIRE(measures == 0);

  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  animator animations;
  auto window = std::make_unique<host_window>("widgets.zero", [&](window_context&) {
    auto root_draw = widgets::stack(container_s, stack_options{}, child);
    widgets::run(root_draw, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window));
  run_frame(*context.ui);
  REQUIRE(draws == 0);
  REQUIRE(measures == 0);

  state flex_zero_s{ "zero.flex" };
  flex_zero_s.set(property_type::width, 0.0f);
  auto flex_zero = widgets::flexbox(flex_zero_s, flex_options{}, child);
  const widgets::layout_size flex_zero_size = flex_zero.measure(appearance, widgets::constraints{ 640.0f, 480.0f });
  REQUIRE(flex_zero_size.width == Catch::Approx(0.0f));
  REQUIRE(flex_zero_size.height == Catch::Approx(0.0f));
  REQUIRE(measures == 0);

  state fixed_s{ "zero.fixed" };
  fixed_s.set(property_type::width, 0.0f);
  fixed_s.set(property_type::height, 100.0f);
  auto fixed = widgets::stack(fixed_s, stack_options{});
  const widgets::layout_size fixed_size = fixed.measure(appearance, widgets::constraints{});
  REQUIRE(fixed_size.width == Catch::Approx(0.0f));
  REQUIRE(fixed_size.height == Catch::Approx(100.0f));
}

TEST_CASE("widgets flexbox grows and shrinks children", "[ui][widgets][flexbox]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state row_s{ "flex.row" };
  row_s.set(property_type::width, 300.0f);
  row_s.set(property_type::height, 40.0f);
  state a_s{ "flex.a" };
  state b_s{ "flex.b" };
  a_s.set(property_type::width, 50.0f);
  a_s.set(property_type::height, 20.0f);
  a_s.set(property_type::flex_grow, 1.0f);
  b_s.set(property_type::width, 50.0f);
  b_s.set(property_type::height, 20.0f);
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.flex", [&](window_context&) {
    auto a = widgets::text(a_s, "A");
    auto b = widgets::text(b_s, "B");
    auto row = widgets::flexbox(
        row_s,
        flex_options{ .direction = flex_direction::row, .gap = 10.0f, .padding = insets{ 5.0f, 5.0f, 5.0f, 5.0f } },
        a,
        b);
    widgets::run(row, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window));
  run_frame(*context.ui);

  REQUIRE(a_s.bounds().width == Catch::Approx(230.0f));
  REQUIRE(b_s.bounds().width == Catch::Approx(50.0f));
  REQUIRE(a_s.bounds().x == Catch::Approx(5.0f));
  REQUIRE(b_s.bounds().x == Catch::Approx(245.0f));
  REQUIRE(row_s.bounds().width == Catch::Approx(300.0f));

  state shrink_s{ "flex.shrink" };
  shrink_s.set(property_type::width, 100.0f);
  shrink_s.set(property_type::height, 20.0f);
  state c_s{ "flex.c" };
  state d_s{ "flex.d" };
  c_s.set(property_type::width, 100.0f);
  c_s.set(property_type::height, 10.0f);
  d_s.set(property_type::width, 100.0f);
  d_s.set(property_type::height, 10.0f);

  auto window2 = std::make_unique<host_window>("widgets.shrink", [&](window_context&) {
    auto c = widgets::text(c_s, "C");
    auto d = widgets::text(d_s, "D");
    auto row = widgets::flexbox(shrink_s, flex_options{}, c, d);
    widgets::run(row, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window2));
  run_frame(*context.ui);
  REQUIRE(c_s.bounds().width == Catch::Approx(50.0f));
  REQUIRE(d_s.bounds().width == Catch::Approx(50.0f));

  state column_s{ "flex.column" };
  column_s.set(property_type::width, 100.0f);
  column_s.set(property_type::height, 200.0f);
  state e_s{ "flex.e" };
  state f_s{ "flex.f" };
  e_s.set(property_type::height, 50.0f);
  f_s.set(property_type::height, 50.0f);

  auto window3 = std::make_unique<host_window>("widgets.column", [&](window_context&) {
    auto e = widgets::text(e_s, "E");
    auto f = widgets::text(f_s, "F");
    auto column = widgets::flexbox(
        column_s, flex_options{ .direction = flex_direction::column, .align = align_items::stretch }, e, f);
    widgets::run(column, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window3));
  run_frame(*context.ui);
  REQUIRE(e_s.bounds().width == Catch::Approx(100.0f));
  REQUIRE(f_s.bounds().width == Catch::Approx(100.0f));
  REQUIRE(e_s.bounds().height == Catch::Approx(50.0f));
  REQUIRE(f_s.bounds().height == Catch::Approx(50.0f));
}

TEST_CASE("widgets run ticks the root before measuring", "[ui][widgets][run]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state root_s{ "root" };
  root_s.set(property_type::height, 100.0f);
  root_s.animate(
      property_type::height, 0.0f, { .duration = 1.0f, .easing = cyberverse::animation::easing_type::linear });
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.run", [&](window_context&) {
    auto root = widgets::stack(root_s, stack_options{});
    widgets::run(root, frame_context{ 0.5f, appearance, animations });
  });
  context.ui->add_window(std::move(window));

  run_frame(*context.ui);
  REQUIRE(root_s.bounds().height == Catch::Approx(50.0f));
  run_frame(*context.ui);
  REQUIRE(root_s.bounds().height == Catch::Approx(0.0f));
}

TEST_CASE("widgets empty stack uses padding", "[ui][widgets][stack]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state root_s{ "root" };
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.empty", [&](window_context&) {
    auto root = widgets::stack(root_s, stack_options{ .padding = insets{ 3.0f, 4.0f, 5.0f, 6.0f } });
    widgets::run(root, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window));

  run_frame(*context.ui);
  REQUIRE(root_s.bounds().width == Catch::Approx(8.0f));
  REQUIRE(root_s.bounds().height == Catch::Approx(10.0f));
}

TEST_CASE("widgets text, button, and stack run with an explicit style override", "[ui][widgets][layout]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state row_s{ "row" };
  state title_s{ "title" };
  state ok_s{ "ok" };
  state plain_s{ "plain" };
  theme appearance;
  animator animations;

  button_style override_style;
  override_style.padding_x = 20.0f;
  override_style.padding_y = 7.0f;

  rect row_bounds{};
  rect title_bounds{};
  rect ok_bounds{};
  ImVec2 title_text_size{};
  ImVec2 ok_text_size{};
  widgets::layout_size plain_button_size{};

  auto window = std::make_unique<host_window>("widgets.layout", [&](window_context&) {
    title_text_size = ImGui::CalcTextSize("Hello");
    ok_text_size = ImGui::CalcTextSize("OK");

    auto title = widgets::text(title_s, "Hello");
    auto ok = widgets::button(ok_s, "OK", override_style);
    auto plain = widgets::button(plain_s, "OK");
    plain_button_size = plain.measure(appearance, widgets::constraints{});
    auto row = widgets::stack(
        row_s,
        stack_options{ .direction = stack_direction::horizontal, .gap = 5.0f, .padding = insets{ 5, 5, 5, 5 } },
        title,
        ok);

    widgets::run(row, frame_context{ frame_delta, appearance, animations });
    row_bounds = row_s.bounds();
    title_bounds = title_s.bounds();
    ok_bounds = ok_s.bounds();
  });
  context.ui->add_window(std::move(window));

  run_frame(*context.ui);

  const float expected_button_width = ok_text_size.x + 40.0f;
  REQUIRE(plain_button_size.width == Catch::Approx(ok_text_size.x + 16.0f));
  REQUIRE(title_bounds.width == Catch::Approx(title_text_size.x));
  REQUIRE(ok_bounds.width == Catch::Approx(expected_button_width));
  REQUIRE(row_bounds.width == Catch::Approx(5.0f + title_bounds.width + 5.0f + ok_bounds.width + 5.0f));
  REQUIRE(row_bounds.height == Catch::Approx(5.0f + std::max(title_bounds.height, ok_bounds.height) + 5.0f));
}

TEST_CASE("widgets button click emits the state event", "[ui][widgets][button]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state row_s{ "row" };
  state ok_s{ "ok" };
  theme appearance;
  animator animations;
  int clicks = 0;
  ok_s.on_event([&](state&, event_type event) {
    if (event == event_type::click) {
      ++clicks;
    }
  });

  rect ok_bounds{};
  auto window = std::make_unique<host_window>("widgets.click", [&](window_context&) {
    auto ok = widgets::button(ok_s, "OK");
    auto row = widgets::stack(row_s, stack_options{}, ok);
    widgets::run(row, frame_context{ frame_delta, appearance, animations });
    ok_bounds = ok_s.bounds();
  });
  context.ui->add_window(std::move(window));

  run_frame(*context.ui);
  REQUIRE(clicks == 0);
  click(context.headless_window(), *context.ui, ok_bounds);
  REQUIRE(clicks == 1);
}

TEST_CASE("widgets checkbox and radio update external values", "[ui][widgets][choice]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state column_s{ "choice.column" };
  state check_s{ "choice.check" };
  state first_s{ "choice.first" };
  state second_s{ "choice.second" };
  bool checked = false;
  int selected = 0;
  int checkbox_events = 0;
  int radio_events = 0;
  theme appearance;
  animator animations;

  check_s.on_event([&](state&, event_type event) {
    if (event == event_type::value_changed) {
      ++checkbox_events;
    }
  });
  second_s.on_event([&](state&, event_type event) {
    if (event == event_type::value_changed) {
      ++radio_events;
    }
  });

  rect check_bounds{};
  rect second_bounds{};
  auto window = std::make_unique<host_window>("widgets.choice", [&](window_context&) {
    auto check = widgets::checkbox(check_s, "Enabled", checked);
    auto first = widgets::radio(first_s, "First", selected, 1);
    auto second = widgets::radio(second_s, "Second", selected, 2);
    auto column = widgets::stack(column_s, stack_options{}, check, first, second);
    widgets::run(column, frame_context{ frame_delta, appearance, animations });
    check_bounds = check_s.bounds();
    second_bounds = second_s.bounds();
  });
  context.ui->add_window(std::move(window));

  run_frame(*context.ui);
  REQUIRE(check_bounds.width > 0.0f);
  click(context.headless_window(), *context.ui, check_bounds);
  REQUIRE(checked);
  REQUIRE(checkbox_events == 1);

  click(context.headless_window(), *context.ui, second_bounds);
  REQUIRE(selected == 2);
  REQUIRE(radio_events == 1);
}

TEST_CASE("widgets text input edits external text", "[ui][widgets][input]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  context.ui->io().ConfigInputTrickleEventQueue = false;

  state column_s{ "input.column" };
  state input_s{ "input.field" };
  std::string value;
  int changed = 0;
  theme appearance;
  animator animations;
  input_s.on_event([&](state&, event_type event) {
    if (event == event_type::value_changed) {
      ++changed;
    }
  });

  rect input_bounds{};
  auto window = std::make_unique<host_window>("widgets.input", [&](window_context&) {
    auto input = widgets::text_input(input_s, value, { .placeholder = "Type" });
    auto column = widgets::stack(column_s, stack_options{}, input);
    widgets::run(column, frame_context{ frame_delta, appearance, animations });
    input_bounds = input_s.bounds();
  });
  context.ui->add_window(std::move(window));

  run_frame(*context.ui);
  click(context.headless_window(), *context.ui, input_bounds);
  context.headless_window().inject_text("abc");
  context.headless_window().poll_events();
  run_frame(*context.ui);

  REQUIRE(value == "abc");
  REQUIRE(changed >= 1);
}

TEST_CASE("widgets divider measures along the available axis", "[ui][widgets][divider]") {
  state divider_s{ "divider" };
  theme appearance;

  auto horizontal =
      widgets::divider(divider_s, divider_options{ .orientation = divider_orientation::horizontal, .thickness = 3.0f });
  const widgets::layout_size horizontal_size = horizontal.measure(appearance, widgets::constraints{ 200.0f, 100.0f });
  REQUIRE(horizontal_size.width == Catch::Approx(200.0f));
  REQUIRE(horizontal_size.height == Catch::Approx(3.0f));

  divider_s.set(property_type::width, 50.0f);
  const widgets::layout_size explicit_size = horizontal.measure(appearance, widgets::constraints{ 200.0f, 100.0f });
  REQUIRE(explicit_size.width == Catch::Approx(50.0f));
  REQUIRE(explicit_size.height == Catch::Approx(3.0f));

  state vertical_s{ "divider.vertical" };
  auto vertical =
      widgets::divider(vertical_s, divider_options{ .orientation = divider_orientation::vertical, .thickness = 2.0f });
  const widgets::layout_size vertical_size = vertical.measure(appearance, widgets::constraints{ 200.0f, 100.0f });
  REQUIRE(vertical_size.width == Catch::Approx(2.0f));
  REQUIRE(vertical_size.height == Catch::Approx(100.0f));

  state zero_s{ "divider.zero" };
  auto zero =
      widgets::divider(zero_s, divider_options{ .orientation = divider_orientation::horizontal, .thickness = 0.0f });
  const widgets::layout_size zero_size = zero.measure(appearance, widgets::constraints{ 200.0f, 100.0f });
  REQUIRE(zero_size.height == Catch::Approx(0.0f));
}

TEST_CASE("widgets scroll_view scrolls both axes and clamps offsets", "[ui][widgets][scroll]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  context.ui->io().ConfigInputTrickleEventQueue = false;

  state view_s{ "scroll.view" };
  view_s.set(property_type::width, 100.0f);
  view_s.set(property_type::height, 100.0f);
  state content_s{ "scroll.content" };
  content_s.set(property_type::width, 300.0f);
  content_s.set(property_type::height, 200.0f);
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.scroll", [&](window_context&) {
    auto content = widgets::text(content_s, "content");
    auto view = widgets::scroll_view(view_s, scroll_options{}, content);
    widgets::run(view, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window));

  view_s.set(property_type::scroll_x, 999.0f);
  view_s.set(property_type::scroll_y, 999.0f);
  run_frame(*context.ui);
  REQUIRE(view_s.value(property_type::scroll_x) == Catch::Approx(200.0f));
  REQUIRE(view_s.value(property_type::scroll_y) == Catch::Approx(100.0f));

  view_s.set(property_type::scroll_x, 0.0f);
  view_s.set(property_type::scroll_y, 0.0f);
  auto& headless = context.headless_window();
  headless.inject_mouse_move(50.0f, 50.0f);
  headless.inject_mouse_scroll(-1.0f, -1.0f);
  headless.poll_events();
  run_frame(*context.ui);
  REQUIRE(view_s.value(property_type::scroll_x) > 0.0f);
  REQUIRE(view_s.value(property_type::scroll_y) > 0.0f);
}

TEST_CASE("widgets scroll_view supports horizontal content", "[ui][widgets][scroll]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state view_s{ "scroll.horizontal" };
  view_s.set(property_type::width, 100.0f);
  view_s.set(property_type::height, 100.0f);
  state first_s{ "scroll.horizontal.first" };
  state second_s{ "scroll.horizontal.second" };
  first_s.set(property_type::width, 80.0f);
  first_s.set(property_type::height, 20.0f);
  second_s.set(property_type::width, 80.0f);
  second_s.set(property_type::height, 20.0f);
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.hscroll", [&](window_context&) {
    auto first = widgets::text(first_s, "A");
    auto second = widgets::text(second_s, "B");
    auto view = widgets::scroll_view(
        view_s, scroll_options{ .direction = scroll_content_direction::horizontal }, first, second);
    widgets::run(view, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window));

  view_s.set(property_type::scroll_x, 999.0f);
  run_frame(*context.ui);
  REQUIRE(view_s.value(property_type::scroll_x) == Catch::Approx(60.0f));
  REQUIRE(view_s.value(property_type::scroll_y) == Catch::Approx(0.0f));
  REQUIRE(first_s.bounds().x == Catch::Approx(-60.0f));
  REQUIRE(second_s.bounds().x == Catch::Approx(20.0f));
}

TEST_CASE("widgets scroll_view scrollbar changes the offset", "[ui][widgets][scroll]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state view_s{ "scroll.bar" };
  view_s.set(property_type::width, 100.0f);
  view_s.set(property_type::height, 100.0f);
  state content_s{ "scroll.bar.content" };
  content_s.set(property_type::width, 300.0f);
  content_s.set(property_type::height, 200.0f);
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.scrollbar", [&](window_context&) {
    auto content = widgets::text(content_s, "content");
    auto view = widgets::scroll_view(view_s, scroll_options{}, content);
    widgets::run(view, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window));
  run_frame(*context.ui);

  const rect view_bounds = view_s.bounds();
  const rect bar_target{ view_bounds.x + view_bounds.width - 2.0f, view_bounds.y + 70.0f, 2.0f, 20.0f };
  click(context.headless_window(), *context.ui, bar_target);
  REQUIRE(view_s.value(property_type::scroll_y) > 0.0f);
}

TEST_CASE("widgets zero-height scroll_view skips content", "[ui][widgets][scroll][zero]") {
  state view_s{ "scroll.zero" };
  view_s.set(property_type::width, 100.0f);
  view_s.set(property_type::height, 0.0f);
  state child_s{ "scroll.zero.child" };
  int measures = 0;
  auto child = widgets::component<probe_impl>{ child_s, probe_impl{ nullptr, nullptr, &measures } };
  theme appearance;
  auto view = widgets::scroll_view(view_s, scroll_options{}, child);
  const widgets::layout_size measured = view.measure(appearance, widgets::constraints{ 640.0f, 480.0f });
  REQUIRE(measured.width == Catch::Approx(100.0f));
  REQUIRE(measured.height == Catch::Approx(0.0f));
  REQUIRE(measures == 0);
}

TEST_CASE("widgets flexbox justifies remaining main-axis space", "[ui][widgets][flexbox]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state row_s{ "flex.justify" };
  row_s.set(property_type::width, 200.0f);
  row_s.set(property_type::height, 20.0f);
  state a_s{ "flex.justify.a" };
  state b_s{ "flex.justify.b" };
  a_s.set(property_type::width, 50.0f);
  a_s.set(property_type::height, 10.0f);
  b_s.set(property_type::width, 50.0f);
  b_s.set(property_type::height, 10.0f);
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.flex.justify", [&](window_context&) {
    auto a = widgets::text(a_s, "A");
    auto b = widgets::text(b_s, "B");
    auto row = widgets::flexbox(row_s, flex_options{ .justify = justify_content::end }, a, b);
    widgets::run(row, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window));
  run_frame(*context.ui);

  REQUIRE(a_s.bounds().x == Catch::Approx(100.0f));
  REQUIRE(b_s.bounds().x == Catch::Approx(150.0f));
}

TEST_CASE("widgets flexbox consumes animated width inputs", "[ui][widgets][flexbox]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state row_s{ "flex.animated" };
  row_s.set(property_type::width, 200.0f);
  row_s.set(property_type::height, 40.0f);
  state a_s{ "flex.animated.a" };
  state b_s{ "flex.animated.b" };
  a_s.set(property_type::width, 50.0f);
  a_s.set(property_type::height, 20.0f);
  b_s.set(property_type::width, 50.0f);
  b_s.set(property_type::height, 20.0f);
  a_s.animate(property_type::width, 150.0f, { .duration = 1.0f, .easing = cyberverse::animation::easing_type::linear });
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.flex.animated", [&](window_context&) {
    auto a = widgets::text(a_s, "A");
    auto b = widgets::text(b_s, "B");
    auto row = widgets::flexbox(row_s, flex_options{}, a, b);
    widgets::run(row, frame_context{ 0.5f, appearance, animations });
  });
  context.ui->add_window(std::move(window));

  run_frame(*context.ui);
  REQUIRE(a_s.value(property_type::width) == Catch::Approx(100.0f));
  REQUIRE(a_s.bounds().width == Catch::Approx(100.0f));
  REQUIRE(b_s.bounds().width == Catch::Approx(50.0f));
}

TEST_CASE("widgets stack distinguishes invisible and collapsed children", "[ui][widgets][visibility]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state row_s{ "row" };
  state first_s{ "first" };
  state second_s{ "second" };
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.visibility", [&](window_context&) {
    auto first = widgets::text(first_s, "A");
    auto second = widgets::text(second_s, "B");
    auto row = widgets::stack(
        row_s,
        stack_options{ .direction = stack_direction::horizontal, .gap = 10.0f, .padding = insets{ 10, 10, 10, 10 } },
        first,
        second);
    widgets::run(row, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window));

  run_frame(*context.ui);
  const float visible_width = row_s.bounds().width;
  const float first_width = first_s.bounds().width;
  const float second_width = second_s.bounds().width;

  second_s.set_visibility(visibility::invisible);
  run_frame(*context.ui);
  REQUIRE(row_s.bounds().width == Catch::Approx(visible_width));
  REQUIRE(second_s.bounds().width == Catch::Approx(second_width));

  second_s.set_visibility(visibility::collapsed);
  run_frame(*context.ui);
  REQUIRE(row_s.bounds().width == Catch::Approx(10.0f + first_width + 10.0f));
  REQUIRE(second_s.bounds().width == Catch::Approx(0.0f));
}

TEST_CASE("widgets font_size changes text layout", "[ui][widgets][font]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));

  state row_s{ "row" };
  state default_s{ "default" };
  state large_s{ "large" };
  large_s.set(property_type::font_size, 28.0f);
  theme appearance;
  animator animations;

  auto window = std::make_unique<host_window>("widgets.font", [&](window_context&) {
    auto normal = widgets::text(default_s, "A");
    auto large = widgets::text(large_s, "A");
    auto row = widgets::stack(row_s, stack_options{}, normal, large);
    widgets::run(row, frame_context{ frame_delta, appearance, animations });
  });
  context.ui->add_window(std::move(window));

  run_frame(*context.ui);
  REQUIRE(large_s.bounds().height > default_s.bounds().height);
  REQUIRE(large_s.bounds().height == Catch::Approx(28.0f));
}
