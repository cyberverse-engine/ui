# ui

Layered UI runtime for Cyberverse Engine applications. `cyberverse::ui` owns the
single Dear ImGui context, the Diligent ImGui renderer, the input bridge, fonts,
theme, and an ordered stack of UI layers.

This is v0.3: the library hard-depends on Dear ImGui and DiligentEngine. There is
no UI backend interface and no renderer abstraction. The only layout model is
"one managed, full-viewport window per panel"; panel content positions itself
with native Dear ImGui APIs.

## Dependencies

- `cyberverse::runtime::static_lib`
- `cyberverse::platform::static_lib`
- `cyberverse::render_core::static_lib`
- `Diligent-Imgui` (Dear ImGui, shipped through DiligentTools)

`Diligent-Imgui` is a hard requirement. If DiligentEngine is configured without
`DILIGENT_BUILD_TOOLS=ON`, configuration fails with an actionable error instead
of falling back to another backend. Configure with
`-DCYBERVERSE_DILIGENT_DIR=<DiligentEngine source tree>` when the sibling
`../DiligentEngine` checkout is not present.

## Contents

- `ui::system`: owns the ImGui context, Diligent renderer, fonts, theme, input
  bridge, and layer stack. One engine frame opens at most one ImGui frame.
- `ui::layer`: one UI layer with a name, order, visibility, and an ordered
  panel list.
- `ui::panel` / `ui::panel_context`: one content unit and the frame information
  passed to `draw()`.
- `ui::frame_stage`: the only `runtime::frame_stage` adapter. It draws every
  layer in one overlay pass and should be registered after all 3D stages.
- `cyberverse/ui/imgui.hpp`: the explicit Dear ImGui entry point for panel
  implementations and style/escape-hatch code.

## No layout component

`ui` does not provide a flexbox, anchors, stacks, spacers, or any other layout
or widget component. Every panel's window is created by the layer and always
fills the layer's logical viewport (`io.DisplaySize`). A panel draws its content
inside that rectangle using native Dear ImGui APIs:

- `ImGui::SetCursorPos` / `ImGui::Dummy` / `ImGui::Spacing` for positioning and
  whitespace;
- `ImGui::BeginChild` for auto-sized boxes;
- `ImGui::GetBackgroundDrawList()` for a dim backdrop;
- ordinary Dear ImGui widgets for controls.

HUD, diagnostics, menus, and modal behavior are application concepts. The
library only orders layers and manages the panel windows.

## Frame lifecycle

1. `ui::frame_stage::on_render` checks `system::has_visible_content()`.
2. It obtains a `render_core::frame_context` (normally
   `render_device::begin_frame`).
3. `system::begin_frame(frame, dt)` binds the frame's color/depth targets,
   resets the scissor, calls the Diligent renderer's `NewFrame`, feeds platform
   input into ImGui, and calls `ImGui::NewFrame()`.
4. `system::draw()` visits visible layers from the lowest order to the highest,
   creates one fixed full-viewport ImGui window per visible panel, and calls
   `panel::draw(context)`.
5. `system::end_frame()` calls `ImGui::Render()`, submits one
   `RenderDrawData()` pass, resets the scissor, and records
   `WantCaptureMouse` / `WantCaptureKeyboard`.

`begin_frame()` returning false means no drawable frame; do not call `draw()` or
`end_frame()`. `end_frame()` without an active frame is a no-op. Layer and panel
structure may only change between frames; mutating it during a frame throws
`std::logic_error`. A panel exception ends its window and discards the ImGui
frame before propagating.

`wants_mouse()` / `wants_keyboard()` reflect the most recently completed frame,
so applications see one frame of latency. Hide layers or use `no_input` panels
when input must be blocked immediately.

## Ownership and threading

- `ui::system` must be destroyed before the render device and platform window.
  Remove the `ui::frame_stage` from the runtime before destroying the system.
- One process has one Dear ImGui context. Applications must not link another
  ImGui copy or create a second context.
- All calls are render-thread calls. There are no hidden worker threads and no
  asynchronous UI frames.
- Register exactly one `ui::frame_stage`; it must render after 3D and before
  present.

## Layer z-order

Layers are drawn from the lowest `order` to the highest, and ties keep insertion
order. The implementation raises each layer's managed windows above lower
windows through `imgui_internal.h`, isolated in
`src/cyberverse/ui/detail/z_order.cpp`. If a Dear ImGui upgrade removes that
internal API, overlapping layers no longer get a strict z-order guarantee;
applications should then avoid overlapping interactive layers and control input
through visibility and `no_input`.

## Example

```cpp
#include "cyberverse/ui.hpp"
#include "cyberverse/ui/imgui.hpp"

class status_panel final : public cyberverse::ui::panel {
public:
  [[nodiscard]] std::string_view id() const noexcept override { return "app.status"; }
  [[nodiscard]] cyberverse::ui::panel_feature features() const noexcept override {
    return cyberverse::ui::panel_feature::no_input;
  }
  void draw(cyberverse::ui::panel_context& context) override {
    ImGui::SetCursorPos(ImVec2{ 16.0f, 16.0f });
    ImGui::TextUnformatted("Status");
    (void)context;
  }
};

cyberverse::ui::system_config config;
config.device = &device;
config.window = &window;
auto ui = cyberverse::ui::system::create(config);

auto layer = std::make_unique<cyberverse::ui::layer>(
    cyberverse::ui::layer_desc{ .name = "app.status", .order = 100 });
layer->add_panel(std::make_unique<status_panel>());
ui->add_layer(std::move(layer));

auto stage = std::make_unique<cyberverse::ui::frame_stage>(*ui, device);
cyberverse::runtime::frame_stage_options options;
options.render_order = 1000;
options.receive_update = false;
pipeline.add_stage("ui", std::move(stage), ui_clock, options);
```

## Headless mode

`system_config::headless = true` creates no GPU resources. It builds the font
atlas eagerly with a test-only texture identifier and still produces Dear ImGui
draw data, so layer ordering, panel windows, input mapping, and exception cleanup
can be tested without a GPU or window. A headless system may own a platform
headless window to inject deterministic input.

## Build and test

Builds and tests must run outside the repository.

```bash
cmake -S . -B /tmp/cyberverse-ui-test \
  -DCYBERVERSE_DILIGENT_DIR=/path/to/DiligentEngine
cmake --build /tmp/cyberverse-ui-test -j 8
ctest --test-dir /tmp/cyberverse-ui-test --output-on-failure
```

The default suite does not open a window or require Vulkan/MoltenVK. The real
GPU smoke test is opt-in:

```bash
CYBERVERSE_UI_TEST_GPU=1 /tmp/cyberverse-ui-test/test/cyberverse_ui_tests "[gpu]"
```
