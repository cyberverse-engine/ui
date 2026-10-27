/**
 * @file frame_stage.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief runtime::frame_stage adapter for the UI runtime
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/frame_stage.hpp"

#include <stdexcept>
#include <utility>

#include "cyberverse/render_core/frame_context.hpp"
#include "cyberverse/render_core/render_device.hpp"
#include "cyberverse/runtime/frame_budget.hpp"
#include "cyberverse/ui/system.hpp"

namespace cyberverse {
namespace ui {

frame_stage::frame_stage(system& ui, render_core::render_device& device)
    : runtime::frame_stage(false), ui_{ &ui }, device_{ &device } {}

frame_stage::frame_stage(system& ui, frame_provider provider)
    : runtime::frame_stage(false), ui_{ &ui }, provider_{ std::move(provider) } {
  if (!provider_) {
    throw std::invalid_argument("cyberverse::ui::frame_stage: frame provider must not be empty");
  }
}

frame_stage::~frame_stage() = default;

void frame_stage::on_update(double /*dt*/, std::uint64_t /*tick*/, const runtime::frame_budget& /*budget*/) {}

void frame_stage::on_render(double dt, double alpha, const runtime::frame_budget& budget) {
  if (ui_ == nullptr || !ui_->has_visible_content()) {
    return;
  }

  render_core::frame_context frame;
  if (provider_) {
    frame = provider_(dt, alpha, budget);
  } else if (device_ != nullptr) {
    frame = device_->begin_frame(dt, alpha, budget);
  } else {
    return;
  }

  if (!ui_->begin_frame(frame, dt)) {
    return;
  }

  ui_->draw();
  ui_->end_frame();
}

} // namespace ui
} // namespace cyberverse
