/**
 * @file font.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief UI font configuration
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>
#include <filesystem>
#include <string>
#include <vector>

namespace cyberverse {
namespace ui {

/**
 * @brief Predefined glyph range baked into a font atlas
 *
 */
enum class glyph_range {
  default_latin,             ///< Dear ImGui's default Latin range
  chinese_simplified_common, ///< ImGui's common Simplified Chinese range
  custom,                    ///< Use font_config::glyphs
};

/**
 * @brief One font source configured on ui::system
 *
 * The first entry is the default font. Entries with merge == true are merged
 * into the previous successfully loaded font, which is how a CJK fallback is
 * attached to a Latin font.
 *
 */
struct font_config {
  std::string id;                                 ///< Stable identifier used by system::font_loaded()
  std::filesystem::path path;                     ///< Empty means the built-in Dear ImGui font
  float size = 16.0f;                             ///< Rasterized size in logical points
  glyph_range range = glyph_range::default_latin; ///< Preloaded range
  std::vector<char32_t> glyphs;                   ///< Used when range == custom
  bool merge = false;                             ///< Merge into the previous font
  int oversample_h = 1;                           ///< Horizontal oversampling
  int oversample_v = 1;                           ///< Vertical oversampling
  bool pixel_snap = false;                        ///< Snap glyphs to pixels
};

} // namespace ui
} // namespace cyberverse
