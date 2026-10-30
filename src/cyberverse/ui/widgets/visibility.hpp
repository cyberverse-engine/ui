/**
 * @file visibility.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Component visibility modes
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Whether a component participates in layout and drawing
 *
 */
enum class visibility {
  visible,   ///< Participates in layout and is drawn
  invisible, ///< Participates in layout but is not drawn
  collapsed, ///< Does not participate in layout and is not drawn
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
