/**
 * @file architecture_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Public header and type checks for the layered UI runtime
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <type_traits>

#include "cyberverse/ui.hpp"
#include "cyberverse/ui/imgui.hpp"

namespace {

using cyberverse::ui::has_feature;
using cyberverse::ui::panel_feature;

} // namespace

TEST_CASE("ui public headers expose the layered model", "[ui][architecture]") {
  STATIC_REQUIRE(static_cast<std::uint32_t>(panel_feature::none) == 0);
  STATIC_REQUIRE(has_feature(panel_feature::no_input | panel_feature::transparent, panel_feature::no_input));
  STATIC_REQUIRE_FALSE(has_feature(panel_feature::transparent, panel_feature::no_input));
  STATIC_REQUIRE(std::is_abstract_v<cyberverse::ui::panel>);
  SUCCEED();
}
