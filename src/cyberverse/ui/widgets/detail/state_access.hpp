/**
 * @file state_access.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Internal state mutation helpers
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include "cyberverse/ui/widgets/types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

class state;

namespace detail {

/**
 * @brief Layout bridge to private state storage
 *
 */
struct state_access {
  static void set_bounds(state& owner, const rect& value) noexcept;
};

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
