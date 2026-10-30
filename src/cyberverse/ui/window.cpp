/**
 * @file window.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief UI window identity
 * @version 0.4
 * @date 2026-10-31
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/window.hpp"

#include <stdexcept>
#include <utility>

namespace cyberverse {
namespace ui {

window::window(std::string id) : id_{ std::move(id) } {
  if (id_.empty()) {
    throw std::invalid_argument("cyberverse::ui::window: id must not be empty");
  }
  if (id_.find("###") != std::string::npos) {
    throw std::invalid_argument("cyberverse::ui::window: id must not contain \"###\"");
  }
  if (id_.find('\0') != std::string::npos) {
    throw std::invalid_argument("cyberverse::ui::window: id must not contain an embedded null character");
  }
}

} // namespace ui
} // namespace cyberverse
