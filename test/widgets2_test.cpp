/**
 * @file widgets2_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief widgets2 state, component, stack, and button tests
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
#include "cyberverse/ui/layer.hpp"
#include "cyberverse/ui/panel.hpp"
#include "cyberverse/ui/panel_context.hpp"
#include "cyberverse/ui/system.hpp"
#include "cyberverse/ui/widgets2.hpp"
#include "cyberverse/ui/widgets2/detail/state_access.hpp"
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::layer;
using cyberverse::ui::layer_desc;
using cyberverse::ui::panel_context;
using cyberverse::ui::system;
using cyberverse::ui::test::make_headless;

namespace w2 = cyberverse::ui::widgets2;
using w2::align_items;
using w2::animator;
using w2::button_style;
using w2::divider_options;
using w2::divider_orientation;
using w2::event_type;
using w2::flex_direction;
using w2::flex_options;
using w2::frame_context;
using w2::insets;
using w2::justify_content;
using w2::property_type;
using w2::rect;
using w2::scroll_content_direction;
using w2::scroll_options;
using w2::stack_direction;
using w2::stack_options;
using w2::state;
using w2::theme;
using w2::update_context;
using w2::visibility;

struct probe_impl {
  float* observed_opacity = nullptr;
  int* draw_count = nullptr;
  int* measure_count = nullptr;

  [[nodiscard]] w2::size measure(const w2::state&, const w2::theme&, const w2::constraints&) const noexcept {
    if (measure_count != nullptr) {
      ++*measure_count;
    }
    return w2::size{ 10.0f, 10.0f };
  }

  void draw(w2::state&, w2::draw_context& ctx) {
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

class host_panel final : public cyberverse::ui::panel {
public:
  explicit host_panel(std::string id, std::function<void(panel_context&)> draw)
      : id_{ std::move(id) }, draw_{ std::move(draw) } {}

  [[nodiscard]] std::string_view id() const noexcept override { return id_; }
  [[nodiscard]] bool visible() const noexcept override { return true; }

  void draw(panel_context& context) override {
    ImGui::SetCursorScreenPos(ImGui::GetWindowPos());
    draw_(context);
  }

private:
  std::string id_;
  std::function<void(panel_context&)> draw_;
};

void run_frame(system& ui) {
  REQUIRE(ui.begin_frame(width, height, frame_delta));
  ui.draw();
  ui.end_frame();
}

void click(cyberverse::platform::backends::headless::window& window, system& ui, const rect& target) {
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

TEST_CASE("widgets2 state stores immediate and animated values", "[ui][widgets2][state]") {
  state value{ "widgets2.state" };
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

TEST_CASE("widgets2 state is non-copyable and non-movable", "[ui][widgets2][state]") {
  STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<state>);
  STATIC_REQUIRE_FALSE(std::is_move_constructible_v<state>);
}

TEST_CASE("widgets2 component applies visibility and opacity centrally", "[ui][widgets2][component]") {
  state value{ "probe" };
  value.set(property_type::opacity, 0.25f);
  theme appearance;
  int draws = 0;
  float observed_opacity = 0.0f;
  w2::component<probe_impl> probe{ value, probe_impl{ &observed_opacity, &draws } };
  w2::detail::state_access::set_bounds(value, rect{ 0.0f, 0.0f, 10.0f, 10.0f });

  w2::draw_context context{ appearance, 1.0f };
  probe.draw(context);
  REQUIRE(draws == 1);
  REQUIRE(observed_opacity == Catch::Approx(0.25f));

  w2::detail::state_access::set_bounds(value, rect{});
  probe.draw(context);
  REQUIRE(draws == 1);
  w2::detail::state_access::set_bounds(value, rect{ 0.0f, 0.0f, 10.0f, 10.0f });

  value.set_visibility(visibility::invisible);
  probe.draw(context);
  REQUIRE(draws == 1);

  value.set_visibility(visibility::collapsed);
  const w2::size collapsed = probe.measure(appearance, w2::constraints{});
  REQUIRE(collapsed.width == Catch::Approx(0.0f));
  REQUIRE(collapsed.height == Catch::Approx(0.0f));
}

TEST_CASE("widgets2 zero-size containers skip child computation", "[ui][widgets2][layout][zero]") {
  state container_s{ "zero.container" };
  state child_s{ "zero.child" };
  container_s.set(property_type::width, 0.0f);

  int measures = 0;
  int draws = 0;
  auto child = w2::component<probe_impl>{ child_s, probe_impl{ nullptr, &draws, &measures } };
  auto root = w2::stack(container_s, stack_options{}, child);

  theme appearance;
  const w2::size measured = root.measure(appearance, w2::constraints{ 640.0f, 480.0f });
  REQUIRE(measured.width == Catch::Approx(0.0f));
  REQUIRE(measured.height == Catch::Approx(0.0f));
  REQUIRE(measures == 0);

  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));
  animator animations;
  auto panel = std::make_unique<host_panel>("widgets2.zero", [&](panel_context&) {
    auto root_draw = w2::stack(container_s, stack_options{}, child);
    w2::run(root_draw, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel));
  run_frame(*context.ui);
  REQUIRE(draws == 0);
  REQUIRE(measures == 0);

  state flex_zero_s{ "zero.flex" };
  flex_zero_s.set(property_type::width, 0.0f);
  auto flex_zero = w2::flexbox(flex_zero_s, flex_options{}, child);
  const w2::size flex_zero_size = flex_zero.measure(appearance, w2::constraints{ 640.0f, 480.0f });
  REQUIRE(flex_zero_size.width == Catch::Approx(0.0f));
  REQUIRE(flex_zero_size.height == Catch::Approx(0.0f));
  REQUIRE(measures == 0);

  state fixed_s{ "zero.fixed" };
  fixed_s.set(property_type::width, 0.0f);
  fixed_s.set(property_type::height, 100.0f);
  auto fixed = w2::stack(fixed_s, stack_options{});
  const w2::size fixed_size = fixed.measure(appearance, w2::constraints{});
  REQUIRE(fixed_size.width == Catch::Approx(0.0f));
  REQUIRE(fixed_size.height == Catch::Approx(100.0f));
}

TEST_CASE("widgets2 flexbox grows and shrinks children", "[ui][widgets2][flexbox]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

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

  auto panel = std::make_unique<host_panel>("widgets2.flex", [&](panel_context&) {
    auto a = w2::text(a_s, "A");
    auto b = w2::text(b_s, "B");
    auto row = w2::flexbox(
        row_s,
        flex_options{ .direction = flex_direction::row, .gap = 10.0f, .padding = insets{ 5.0f, 5.0f, 5.0f, 5.0f } },
        a,
        b);
    w2::run(row, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel));
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

  auto panel2 = std::make_unique<host_panel>("widgets2.shrink", [&](panel_context&) {
    auto c = w2::text(c_s, "C");
    auto d = w2::text(d_s, "D");
    auto row = w2::flexbox(shrink_s, flex_options{}, c, d);
    w2::run(row, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel2));
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

  auto panel3 = std::make_unique<host_panel>("widgets2.column", [&](panel_context&) {
    auto e = w2::text(e_s, "E");
    auto f = w2::text(f_s, "F");
    auto column =
        w2::flexbox(column_s, flex_options{ .direction = flex_direction::column, .align = align_items::stretch }, e, f);
    w2::run(column, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel3));
  run_frame(*context.ui);
  REQUIRE(e_s.bounds().width == Catch::Approx(100.0f));
  REQUIRE(f_s.bounds().width == Catch::Approx(100.0f));
  REQUIRE(e_s.bounds().height == Catch::Approx(50.0f));
  REQUIRE(f_s.bounds().height == Catch::Approx(50.0f));
}

TEST_CASE("widgets2 run ticks the root before measuring", "[ui][widgets2][run]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

  state root_s{ "root" };
  root_s.set(property_type::height, 100.0f);
  root_s.animate(
      property_type::height, 0.0f, { .duration = 1.0f, .easing = cyberverse::animation::easing_type::linear });
  theme appearance;
  animator animations;

  auto panel = std::make_unique<host_panel>("widgets2.run", [&](panel_context&) {
    auto root = w2::stack(root_s, stack_options{});
    w2::run(root, frame_context{ 0.5f, appearance, animations });
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  REQUIRE(root_s.bounds().height == Catch::Approx(50.0f));
  run_frame(*context.ui);
  REQUIRE(root_s.bounds().height == Catch::Approx(0.0f));
}

TEST_CASE("widgets2 empty stack uses padding", "[ui][widgets2][stack]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

  state root_s{ "root" };
  theme appearance;
  animator animations;

  auto panel = std::make_unique<host_panel>("widgets2.empty", [&](panel_context&) {
    auto root = w2::stack(root_s, stack_options{ .padding = insets{ 3.0f, 4.0f, 5.0f, 6.0f } });
    w2::run(root, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  REQUIRE(root_s.bounds().width == Catch::Approx(8.0f));
  REQUIRE(root_s.bounds().height == Catch::Approx(10.0f));
}

TEST_CASE("widgets2 text, button, and stack run with an explicit style override", "[ui][widgets2][layout]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

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
  w2::size plain_button_size{};

  auto panel = std::make_unique<host_panel>("widgets2.layout", [&](panel_context&) {
    title_text_size = ImGui::CalcTextSize("Hello");
    ok_text_size = ImGui::CalcTextSize("OK");

    auto title = w2::text(title_s, "Hello");
    auto ok = w2::button(ok_s, "OK", override_style);
    auto plain = w2::button(plain_s, "OK");
    plain_button_size = plain.measure(appearance, w2::constraints{});
    auto row = w2::stack(
        row_s,
        stack_options{ .direction = stack_direction::horizontal, .gap = 5.0f, .padding = insets{ 5, 5, 5, 5 } },
        title,
        ok);

    w2::run(row, frame_context{ frame_delta, appearance, animations });
    row_bounds = row_s.bounds();
    title_bounds = title_s.bounds();
    ok_bounds = ok_s.bounds();
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);

  const float expected_button_width = ok_text_size.x + 40.0f;
  REQUIRE(plain_button_size.width == Catch::Approx(ok_text_size.x + 16.0f));
  REQUIRE(title_bounds.width == Catch::Approx(title_text_size.x));
  REQUIRE(ok_bounds.width == Catch::Approx(expected_button_width));
  REQUIRE(row_bounds.width == Catch::Approx(5.0f + title_bounds.width + 5.0f + ok_bounds.width + 5.0f));
  REQUIRE(row_bounds.height == Catch::Approx(5.0f + std::max(title_bounds.height, ok_bounds.height) + 5.0f));
}

TEST_CASE("widgets2 button click emits the state event", "[ui][widgets2][button]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

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
  auto panel = std::make_unique<host_panel>("widgets2.click", [&](panel_context&) {
    auto ok = w2::button(ok_s, "OK");
    auto row = w2::stack(row_s, stack_options{}, ok);
    w2::run(row, frame_context{ frame_delta, appearance, animations });
    ok_bounds = ok_s.bounds();
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  REQUIRE(clicks == 0);
  click(context.headless_window(), *context.ui, ok_bounds);
  REQUIRE(clicks == 1);
}

TEST_CASE("widgets2 checkbox and radio update external values", "[ui][widgets2][choice]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

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
  auto panel = std::make_unique<host_panel>("widgets2.choice", [&](panel_context&) {
    auto check = w2::checkbox(check_s, "Enabled", checked);
    auto first = w2::radio(first_s, "First", selected, 1);
    auto second = w2::radio(second_s, "Second", selected, 2);
    auto column = w2::stack(column_s, stack_options{}, check, first, second);
    w2::run(column, frame_context{ frame_delta, appearance, animations });
    check_bounds = check_s.bounds();
    second_bounds = second_s.bounds();
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  REQUIRE(check_bounds.width > 0.0f);
  click(context.headless_window(), *context.ui, check_bounds);
  REQUIRE(checked);
  REQUIRE(checkbox_events == 1);

  click(context.headless_window(), *context.ui, second_bounds);
  REQUIRE(selected == 2);
  REQUIRE(radio_events == 1);
}

TEST_CASE("widgets2 text input edits external text", "[ui][widgets2][input]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  context.ui->io().ConfigInputTrickleEventQueue = false;
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

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
  auto panel = std::make_unique<host_panel>("widgets2.input", [&](panel_context&) {
    auto input = w2::text_input(input_s, value, { .placeholder = "Type" });
    auto column = w2::stack(column_s, stack_options{}, input);
    w2::run(column, frame_context{ frame_delta, appearance, animations });
    input_bounds = input_s.bounds();
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  click(context.headless_window(), *context.ui, input_bounds);
  context.headless_window().inject_text("abc");
  context.headless_window().poll_events();
  run_frame(*context.ui);

  REQUIRE(value == "abc");
  REQUIRE(changed >= 1);
}

TEST_CASE("widgets2 divider measures along the available axis", "[ui][widgets2][divider]") {
  state divider_s{ "divider" };
  theme appearance;

  auto horizontal =
      w2::divider(divider_s, divider_options{ .orientation = divider_orientation::horizontal, .thickness = 3.0f });
  const w2::size horizontal_size = horizontal.measure(appearance, w2::constraints{ 200.0f, 100.0f });
  REQUIRE(horizontal_size.width == Catch::Approx(200.0f));
  REQUIRE(horizontal_size.height == Catch::Approx(3.0f));

  divider_s.set(property_type::width, 50.0f);
  const w2::size explicit_size = horizontal.measure(appearance, w2::constraints{ 200.0f, 100.0f });
  REQUIRE(explicit_size.width == Catch::Approx(50.0f));
  REQUIRE(explicit_size.height == Catch::Approx(3.0f));

  state vertical_s{ "divider.vertical" };
  auto vertical =
      w2::divider(vertical_s, divider_options{ .orientation = divider_orientation::vertical, .thickness = 2.0f });
  const w2::size vertical_size = vertical.measure(appearance, w2::constraints{ 200.0f, 100.0f });
  REQUIRE(vertical_size.width == Catch::Approx(2.0f));
  REQUIRE(vertical_size.height == Catch::Approx(100.0f));

  state zero_s{ "divider.zero" };
  auto zero = w2::divider(zero_s, divider_options{ .orientation = divider_orientation::horizontal, .thickness = 0.0f });
  const w2::size zero_size = zero.measure(appearance, w2::constraints{ 200.0f, 100.0f });
  REQUIRE(zero_size.height == Catch::Approx(0.0f));
}

TEST_CASE("widgets2 scroll_view scrolls both axes and clamps offsets", "[ui][widgets2][scroll]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  context.ui->io().ConfigInputTrickleEventQueue = false;
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

  state view_s{ "scroll.view" };
  view_s.set(property_type::width, 100.0f);
  view_s.set(property_type::height, 100.0f);
  state content_s{ "scroll.content" };
  content_s.set(property_type::width, 300.0f);
  content_s.set(property_type::height, 200.0f);
  theme appearance;
  animator animations;

  auto panel = std::make_unique<host_panel>("widgets2.scroll", [&](panel_context&) {
    auto content = w2::text(content_s, "content");
    auto view = w2::scroll_view(view_s, scroll_options{}, content);
    w2::run(view, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel));

  view_s.set(property_type::scroll_x, 999.0f);
  view_s.set(property_type::scroll_y, 999.0f);
  run_frame(*context.ui);
  REQUIRE(view_s.value(property_type::scroll_x) == Catch::Approx(200.0f));
  REQUIRE(view_s.value(property_type::scroll_y) == Catch::Approx(100.0f));

  view_s.set(property_type::scroll_x, 0.0f);
  view_s.set(property_type::scroll_y, 0.0f);
  auto& window = context.headless_window();
  window.inject_mouse_move(50.0f, 50.0f);
  window.inject_mouse_scroll(-1.0f, -1.0f);
  window.poll_events();
  run_frame(*context.ui);
  REQUIRE(view_s.value(property_type::scroll_x) > 0.0f);
  REQUIRE(view_s.value(property_type::scroll_y) > 0.0f);
}

TEST_CASE("widgets2 scroll_view supports horizontal content", "[ui][widgets2][scroll]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

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

  auto panel = std::make_unique<host_panel>("widgets2.hscroll", [&](panel_context&) {
    auto first = w2::text(first_s, "A");
    auto second = w2::text(second_s, "B");
    auto view =
        w2::scroll_view(view_s, scroll_options{ .direction = scroll_content_direction::horizontal }, first, second);
    w2::run(view, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel));

  view_s.set(property_type::scroll_x, 999.0f);
  run_frame(*context.ui);
  REQUIRE(view_s.value(property_type::scroll_x) == Catch::Approx(60.0f));
  REQUIRE(view_s.value(property_type::scroll_y) == Catch::Approx(0.0f));
  REQUIRE(first_s.bounds().x == Catch::Approx(-60.0f));
  REQUIRE(second_s.bounds().x == Catch::Approx(20.0f));
}

TEST_CASE("widgets2 scroll_view scrollbar changes the offset", "[ui][widgets2][scroll]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

  state view_s{ "scroll.bar" };
  view_s.set(property_type::width, 100.0f);
  view_s.set(property_type::height, 100.0f);
  state content_s{ "scroll.bar.content" };
  content_s.set(property_type::width, 300.0f);
  content_s.set(property_type::height, 200.0f);
  theme appearance;
  animator animations;

  auto panel = std::make_unique<host_panel>("widgets2.scrollbar", [&](panel_context&) {
    auto content = w2::text(content_s, "content");
    auto view = w2::scroll_view(view_s, scroll_options{}, content);
    w2::run(view, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel));
  run_frame(*context.ui);

  const rect view_bounds = view_s.bounds();
  const rect bar_target{ view_bounds.x + view_bounds.width - 2.0f, view_bounds.y + 70.0f, 2.0f, 20.0f };
  click(context.headless_window(), *context.ui, bar_target);
  REQUIRE(view_s.value(property_type::scroll_y) > 0.0f);
}

TEST_CASE("widgets2 zero-height scroll_view skips content", "[ui][widgets2][scroll][zero]") {
  state view_s{ "scroll.zero" };
  view_s.set(property_type::width, 100.0f);
  view_s.set(property_type::height, 0.0f);
  state child_s{ "scroll.zero.child" };
  int measures = 0;
  auto child = w2::component<probe_impl>{ child_s, probe_impl{ nullptr, nullptr, &measures } };
  theme appearance;
  auto view = w2::scroll_view(view_s, scroll_options{}, child);
  const w2::size measured = view.measure(appearance, w2::constraints{ 640.0f, 480.0f });
  REQUIRE(measured.width == Catch::Approx(100.0f));
  REQUIRE(measured.height == Catch::Approx(0.0f));
  REQUIRE(measures == 0);
}

TEST_CASE("widgets2 flexbox justifies remaining main-axis space", "[ui][widgets2][flexbox]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

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

  auto panel = std::make_unique<host_panel>("widgets2.flex.justify", [&](panel_context&) {
    auto a = w2::text(a_s, "A");
    auto b = w2::text(b_s, "B");
    auto row = w2::flexbox(row_s, flex_options{ .justify = justify_content::end }, a, b);
    w2::run(row, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel));
  run_frame(*context.ui);

  REQUIRE(a_s.bounds().x == Catch::Approx(100.0f));
  REQUIRE(b_s.bounds().x == Catch::Approx(150.0f));
}

TEST_CASE("widgets2 flexbox consumes animated width inputs", "[ui][widgets2][flexbox]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

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

  auto panel = std::make_unique<host_panel>("widgets2.flex.animated", [&](panel_context&) {
    auto a = w2::text(a_s, "A");
    auto b = w2::text(b_s, "B");
    auto row = w2::flexbox(row_s, flex_options{}, a, b);
    w2::run(row, frame_context{ 0.5f, appearance, animations });
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  REQUIRE(a_s.value(property_type::width) == Catch::Approx(100.0f));
  REQUIRE(a_s.bounds().width == Catch::Approx(100.0f));
  REQUIRE(b_s.bounds().width == Catch::Approx(50.0f));
}

TEST_CASE("widgets2 stack distinguishes invisible and collapsed children", "[ui][widgets2][visibility]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

  state row_s{ "row" };
  state first_s{ "first" };
  state second_s{ "second" };
  theme appearance;
  animator animations;

  auto panel = std::make_unique<host_panel>("widgets2.visibility", [&](panel_context&) {
    auto first = w2::text(first_s, "A");
    auto second = w2::text(second_s, "B");
    auto row = w2::stack(
        row_s,
        stack_options{ .direction = stack_direction::horizontal, .gap = 10.0f, .padding = insets{ 10, 10, 10, 10 } },
        first,
        second);
    w2::run(row, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel));

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

TEST_CASE("widgets2 font_size changes text layout", "[ui][widgets2][font]") {
  auto context = make_headless(static_cast<int>(width), static_cast<int>(height));
  auto& layer = context.ui->add_layer(std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "widgets2" }));

  state row_s{ "row" };
  state default_s{ "default" };
  state large_s{ "large" };
  large_s.set(property_type::font_size, 28.0f);
  theme appearance;
  animator animations;

  auto panel = std::make_unique<host_panel>("widgets2.font", [&](panel_context&) {
    auto normal = w2::text(default_s, "A");
    auto large = w2::text(large_s, "A");
    auto row = w2::stack(row_s, stack_options{}, normal, large);
    w2::run(row, frame_context{ frame_delta, appearance, animations });
  });
  layer.add_panel(std::move(panel));

  run_frame(*context.ui);
  REQUIRE(large_s.bounds().height > default_s.bounds().height);
  REQUIRE(large_s.bounds().height == Catch::Approx(28.0f));
}
