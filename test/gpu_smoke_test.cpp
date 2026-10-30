/**
 * @file gpu_smoke_test.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Optional real GPU/window smoke test
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include <catch2/catch_test_macros.hpp>

#include <cstdint>
#include <cstdlib>
#include <memory>
#include <string_view>

#include "cyberverse/platform/platform.hpp"
#include "cyberverse/platform/window.hpp"
#include "cyberverse/render_core/frame_context.hpp"
#include "cyberverse/render_core/render_device.hpp"
#include "cyberverse/runtime/frame_budget.hpp"
#include "cyberverse/ui/imgui.hpp"
#include "cyberverse/ui/layer.hpp"
#include "cyberverse/ui/system.hpp"
#include "cyberverse/ui/window.hpp"

namespace {

using cyberverse::ui::layer;
using cyberverse::ui::layer_desc;
using cyberverse::ui::system;
using cyberverse::ui::system_config;
using cyberverse::ui::window;
using cyberverse::ui::window_context;

class smoke_window final : public window {
public:
  [[nodiscard]] std::string_view id() const noexcept override { return "gpu.smoke"; }

  void draw(window_context& /*context*/) override { ImGui::TextUnformatted("cyberverse ui gpu smoke"); }
};

} // namespace

TEST_CASE("ui renders one overlay pass to a real swap chain", "[ui][gpu]") {
  if (std::getenv("CYBERVERSE_UI_TEST_GPU") == nullptr) {
    SKIP("set CYBERVERSE_UI_TEST_GPU=1 to run the window/GPU UI smoke test");
  }

  auto platform = cyberverse::platform::make_platform(cyberverse::platform::default_backend());
  if (platform->backend_type() != cyberverse::platform::backend_type::glfw) {
    SKIP("the GLFW platform backend is required for the GPU smoke test");
  }

  cyberverse::platform::window_config window_config;
  window_config.width = 320;
  window_config.height = 240;
  window_config.title = "cyberverse-ui-gpu-test";
  window_config.resizable = false;
  auto window = platform->create_window(window_config);
  REQUIRE(window != nullptr);

  cyberverse::render_core::render_device_desc device_desc;
  device_desc.window = window->native_handle();
  device_desc.width = static_cast<std::uint32_t>(window->width());
  device_desc.height = static_cast<std::uint32_t>(window->height());
  device_desc.vsync = false;
  device_desc.debug_name = "Cyberverse UI GPU Test";
  auto device = cyberverse::render_core::render_device::create(device_desc);
  REQUIRE(device != nullptr);

  {
    system_config config;
    config.device = device.get();
    config.window = window.get();
    config.fonts = { cyberverse::ui::font_config{ .id = "default", .size = 16.0f } };
    auto ui = system::create(config);

    auto layer = std::make_unique<cyberverse::ui::layer>(layer_desc{ .name = "gpu.layer" });
    layer->add_window(std::make_unique<smoke_window>());
    ui->add_layer(std::move(layer));

    const auto frame = device->begin_frame(1.0 / 60.0);
    REQUIRE(ui->begin_frame(frame, 1.0 / 60.0));
    ui->draw();
    ui->end_frame();
    device->present(false);
    device->wait_idle();
    REQUIRE(device->frame_index() == 1);
  }
}
