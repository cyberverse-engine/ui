/**
 * @file architecture_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Public header and type checks for the functional UI runtime
 * @version 0.4
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <type_traits>

#include "cyberverse/ui.hpp"
#include "cyberverse/ui/imgui.hpp"
#include "cyberverse/ui/widgets.hpp"

namespace {

using cyberverse::ui::has_feature;
using cyberverse::ui::window_feature;

} // namespace

TEST_CASE("ui public headers expose the window model", "[ui][architecture]") {
  STATIC_REQUIRE(static_cast<std::uint32_t>(window_feature::none) == 0);
  STATIC_REQUIRE(has_feature(window_feature::no_input | window_feature::transparent, window_feature::no_input));
  STATIC_REQUIRE_FALSE(has_feature(window_feature::transparent, window_feature::no_input));
  STATIC_REQUIRE(std::is_abstract_v<cyberverse::ui::window>);
  STATIC_REQUIRE_FALSE(std::is_copy_constructible_v<cyberverse::ui::widgets::state>);
  STATIC_REQUIRE_FALSE(std::is_move_constructible_v<cyberverse::ui::widgets::state>);
  STATIC_REQUIRE(std::is_enum_v<cyberverse::ui::widgets::property_type>);
  STATIC_REQUIRE(std::is_standard_layout_v<cyberverse::ui::rect>);
  STATIC_REQUIRE(std::is_standard_layout_v<cyberverse::ui::layout_size>);
  STATIC_REQUIRE(std::is_same_v<cyberverse::ui::rect, cyberverse::ui::widgets::rect>);
  STATIC_REQUIRE(std::is_same_v<cyberverse::ui::layout_size, cyberverse::ui::widgets::layout_size>);
  STATIC_REQUIRE(std::is_class_v<cyberverse::ui::widgets::theme>);
  SUCCEED();
}
