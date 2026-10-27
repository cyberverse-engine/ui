/**
 * @file frame_stage_access.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Internal read-only access to a UI layer's panel list
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <memory>
#include <vector>

#include "cyberverse/ui/frame_stage.hpp"

namespace cyberverse {
namespace ui {
namespace detail {

/**
 * @brief Detail-only accessor used by the system draw loop
 *
 */
class frame_stage_access {
public:
  [[nodiscard]] static const std::vector<std::unique_ptr<panel>>& panels(const frame_stage& layer) noexcept {
    return layer.panels_;
  }
};

} // namespace detail
} // namespace ui
} // namespace cyberverse
