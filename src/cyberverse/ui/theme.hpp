/**
 * @file theme.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief UI theme presets
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

namespace cyberverse {
namespace ui {

/**
 * @brief Built-in Dear ImGui style preset
 *
 * Applications may customize the style through host::style(); calling
 * host::set_theme() afterwards overwrites that customization.
 *
 */
enum class theme_kind {
  dark, ///< Dark neutral preset
  light ///< Light neutral preset
};

} // namespace ui
} // namespace cyberverse
