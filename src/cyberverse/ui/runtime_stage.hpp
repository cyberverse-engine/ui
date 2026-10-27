/**
 * @file runtime_stage.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief runtime::frame_stage adapter for the UI runtime
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <functional>

#include "cyberverse/runtime/frame_stage.hpp"

namespace cyberverse {

namespace render_core {
class frame_context;
class render_device;
} // namespace render_core

namespace ui {

class system;

/**
 * @brief Produces the render frame context used for one UI pass
 *
 * The provider is the test/offscreen seam: an application may share an existing
 * frame context instead of calling render_device::begin_frame() again.
 *
 */
using frame_provider =
    std::function<render_core::frame_context(double delta_time, double alpha, const runtime::frame_budget& budget)>;

/**
 * @brief The only runtime adapter owned by the UI runtime
 *
 * The stage is variable-step and leaves on_update empty. on_render opens one
 * Dear ImGui frame shared by all layers and submits the overlay pass. Register
 * one instance after every 3D render stage with receive_update = false,
 * receive_render = true, and a high render_order.
 *
 * When there is no visible content the stage does not begin a render-device
 * frame and does not open an Dear ImGui frame.
 *
 */
class runtime_stage final : public runtime::frame_stage {
public:
  /**
   * @param ui UI runtime; must outlive the stage
   * @param device Device used to begin one render frame per UI pass
   */
  runtime_stage(system& ui, render_core::render_device& device);

  /**
   * @param ui UI runtime; must outlive the stage
   * @param provider Frame-context provider; must not be empty
   * @throws std::invalid_argument If provider is empty
   */
  runtime_stage(system& ui, frame_provider provider);

  ~runtime_stage() override;

  runtime_stage(const runtime_stage&) = delete;
  runtime_stage(runtime_stage&&) = delete;
  runtime_stage& operator=(const runtime_stage&) = delete;
  runtime_stage& operator=(runtime_stage&&) = delete;

  void on_update(double dt, std::uint64_t tick, const runtime::frame_budget& budget) override;
  void on_render(double dt, double alpha, const runtime::frame_budget& budget) override;

private:
  system* ui_ = nullptr;
  render_core::render_device* device_ = nullptr;
  frame_provider provider_;
};

} // namespace ui
} // namespace cyberverse
