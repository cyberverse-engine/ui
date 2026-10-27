/**
 * @file frame_stage.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief One UI layer implementation
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/frame_stage.hpp"

#include <algorithm>
#include <stdexcept>
#include <string>
#include <utility>

#include "cyberverse/ui/detail/frame_phase.hpp"

namespace cyberverse {
namespace ui {

namespace {

void validate_name(std::string_view name) {
  if (name.empty()) {
    throw std::invalid_argument("cyberverse::ui::frame_stage: name must not be empty");
  }
  if (name.find("###") != std::string_view::npos) {
    throw std::invalid_argument("cyberverse::ui::frame_stage: name must not contain \"###\"");
  }
  if (name.find('/') != std::string_view::npos) {
    throw std::invalid_argument("cyberverse::ui::frame_stage: name must not contain '/'");
  }
}

void validate_panel_id(std::string_view id) {
  if (id.empty()) {
    throw std::invalid_argument("cyberverse::ui::frame_stage: a panel id must not be empty");
  }
  if (id.find("###") != std::string_view::npos) {
    throw std::invalid_argument("cyberverse::ui::frame_stage: a panel id must not contain \"###\"");
  }
  if (id.find('/') != std::string_view::npos) {
    throw std::invalid_argument("cyberverse::ui::frame_stage: a panel id must not contain '/'");
  }
}

} // namespace

frame_stage::frame_stage(frame_stage_desc desc)
    : name_{ std::move(desc.name) }, order_{ desc.order }, visible_{ desc.visible } {
  detail::require_between_frames("create a UI layer");
  validate_name(name_);
}

frame_stage::~frame_stage() = default;

std::string_view frame_stage::name() const noexcept { return name_; }

int frame_stage::order() const noexcept { return order_; }

void frame_stage::set_order(int order) {
  detail::require_between_frames("change the UI layer order");
  order_ = order;
}

bool frame_stage::visible() const noexcept { return visible_; }

void frame_stage::set_visible(bool visible) {
  detail::require_between_frames("change the UI layer visibility");
  visible_ = visible;
}

panel& frame_stage::add_panel(std::unique_ptr<panel> content) {
  detail::require_between_frames("add a UI panel");
  if (content == nullptr) {
    throw std::invalid_argument("cyberverse::ui::frame_stage: panel must not be null");
  }
  const std::string_view id = content->id();
  validate_panel_id(id);
  for (const auto& existing : panels_) {
    if (existing != nullptr && existing->id() == id) {
      throw std::invalid_argument(std::string{ "cyberverse::ui::frame_stage: duplicate panel id '" } +
                                  std::string{ id } + "'");
    }
  }
  panels_.push_back(std::move(content));
  return *panels_.back();
}

bool frame_stage::remove_panel(panel& content) noexcept {
  if (detail::frame_in_progress()) {
    return false;
  }
  const auto iterator = std::find_if(panels_.begin(), panels_.end(), [&content](const auto& candidate) {
    return candidate.get() == &content;
  });
  if (iterator == panels_.end()) {
    return false;
  }
  panels_.erase(iterator);
  return true;
}

void frame_stage::clear_panels() {
  detail::require_between_frames("clear UI panels");
  panels_.clear();
}

bool frame_stage::has_visible_panels() const noexcept {
  return std::any_of(panels_.begin(), panels_.end(), [](const auto& content) {
    return content != nullptr && content->visible();
  });
}

} // namespace ui
} // namespace cyberverse
