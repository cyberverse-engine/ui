/**
 * @file font_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief UI font configuration tests
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <filesystem>
#include <memory>

#include "cyberverse/platform/backends/headless/window.hpp"
#include "cyberverse/platform/platform.hpp"
#include "cyberverse/ui/font.hpp"
#include "cyberverse/ui/host.hpp"

namespace {

using cyberverse::ui::font_config;
using cyberverse::ui::glyph_range;
using cyberverse::ui::host;
using cyberverse::ui::host_config;

host_config headless_config() {
  host_config config;
  config.headless = true;
  return config;
}

} // namespace

TEST_CASE("the configured default font loads in headless mode", "[ui][font]") {
  host_config config = headless_config();
  config.fonts = { font_config{ .id = "default", .size = 18.0f } };

  auto ui = host::create(config);
  REQUIRE(ui->font_loaded("default"));
}

TEST_CASE("missing font files are reported instead of failing creation", "[ui][font]") {
  host_config config = headless_config();
  config.fonts = {
    font_config{ .id = "default", .size = 16.0f },
    font_config{ .id = "cjk",
                 .path = std::filesystem::path{ "/cyberverse/does-not-exist/font.ttf" },
                 .size = 16.0f,
                 .range = glyph_range::chinese_simplified_common,
                 .merge = true },
  };

  auto ui = host::create(config);
  REQUIRE(ui->font_loaded("default"));
  REQUIRE_FALSE(ui->font_loaded("cjk"));
}

TEST_CASE("invalid custom glyph ranges do not break font creation", "[ui][font]") {
  host_config config = headless_config();
  config.fonts = { font_config{
      .id = "custom",
      .size = 16.0f,
      .range = glyph_range::custom,
      .glyphs = { static_cast<char32_t>('A'), static_cast<char32_t>('A'), static_cast<char32_t>('Z') },
  } };

  auto ui = host::create(config);
  REQUIRE(ui->font_loaded("custom"));
}
