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
#include "support/test_ui.hpp"

namespace {

using cyberverse::ui::layer;
using cyberverse::ui::layer_desc;
using cyberverse::ui::panel_context;
using cyberverse::ui::system;
using cyberverse::ui::test::make_headless;

namespace w2 = cyberverse::ui::widgets2;
using w2::animator;
using w2::button_style;
using w2::event_type;
using w2::frame_context;
using w2::insets;
using w2::property_type;
using w2::rect;
using w2::stack_direction;
using w2::stack_options;
using w2::state;
using w2::theme;
using w2::update_context;
using w2::visibility;

struct probe_impl {
  float* observed_opacity = nullptr;
  int* draw_count = nullptr;

  [[nodiscard]] w2::size measure(const w2::state&, const w2::theme&, const w2::constraints&) const noexcept {
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

  w2::draw_context context{ appearance, 1.0f };
  probe.draw(context);
  REQUIRE(draws == 1);
  REQUIRE(observed_opacity == Catch::Approx(0.25f));

  value.set_visibility(visibility::invisible);
  probe.draw(context);
  REQUIRE(draws == 1);

  value.set_visibility(visibility::collapsed);
  const w2::size collapsed = probe.measure(appearance, w2::constraints{});
  REQUIRE(collapsed.width == Catch::Approx(0.0f));
  REQUIRE(collapsed.height == Catch::Approx(0.0f));
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
