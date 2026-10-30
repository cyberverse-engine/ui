/**
 * @file layer_access.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Internal read-only access to a UI layer's window list
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <memory>
#include <vector>

#include "cyberverse/ui/layer.hpp"

namespace cyberverse {
namespace ui {
namespace detail {

/**
 * @brief Detail-only accessor used by the system draw loop
 *
 */
class layer_access {
public:
  [[nodiscard]] static const std::vector<std::unique_ptr<window>>& windows(const layer& layer) noexcept {
    return layer.windows_;
  }
};

} // namespace detail
} // namespace ui
} // namespace cyberverse
