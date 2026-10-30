# ui

Layered UI runtime for Cyberverse Engine applications. `cyberverse::ui` owns the
single Dear ImGui context, the Diligent ImGui renderer, the input bridge, fonts,
theme, and an ordered stack of UI layers.

This is v0.4: the library hard-depends on Dear ImGui and DiligentEngine. There is
no UI backend interface and no renderer abstraction. Every panel is still one
managed, full-viewport ImGui window; panel content may use native Dear ImGui
directly or the optional `ui::widgets` standard component layer.

## Dependencies

- `cyberverse::runtime::static_lib`
- `cyberverse::platform::static_lib`
- `cyberverse::render_core::static_lib`
- `Diligent-Imgui` (Dear ImGui, shipped through DiligentTools)
- [Yoga](https://github.com/facebook/yoga) v3.2.1 (MIT), linked **privately** for
  `ui::widgets` flexbox layout
- `cyberverse::animation::static_lib` for the experimental `widgets2` target

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
- `ui::widgets`: an optional standard component layer built on native Dear ImGui
  controls and Yoga: flexbox, panel, scroll view, collapse group, stack, divider,
  text, button, checkbox, radio, text input, and a native escape hatch. It is optional to use
  from a panel but is always built with the library.
- `cyberverse/ui/imgui.hpp`: the explicit Dear ImGui entry point for panel
  implementations and style/escape-hatch code.
- `widgets2` (experimental): a separate, functional component library with
  caller-owned state, compile-time stack children, no virtual dispatch, and
  fixed-frame `run()` ordering. It is intentionally independent of the
  Yoga-backed `widgets` layer and is not included by `ui.hpp`.

## Panels and the optional widget layer

`ui` does not force a layout model on panels. A panel's window is created by the
layer and always fills the layer's logical viewport (`io.DisplaySize`). A panel
can draw content inside that rectangle with native Dear ImGui APIs:

- `ImGui::SetCursorPos` / `ImGui::Dummy` / `ImGui::Spacing` for positioning and
  whitespace;
- `ImGui::BeginChild` for auto-sized boxes;
- `ImGui::GetBackgroundDrawList()` for a dim backdrop;
- ordinary Dear ImGui widgets for controls.

For applications that want a reusable, flexbox-based component layer, include
`cyberverse/ui/widgets.hpp` and call `ui::widgets::render(root, theme)` inside the
panel. The widget layer is deliberately thin:

- `flexbox` uses Yoga for row/column layout, gap, padding, justify, align,
  grow/shrink, and fit/stretch sizing;
- `panel`, `scroll_view`, and `collapse_group` are containers;
- `stack` is a no-wrap horizontal or vertical container whose width and height are
  automatically sized to its content; explicit width/height can override that;
- `divider` is a lightweight horizontal or vertical line with fixed thickness and
  stretch/cross-axis filling;
- `text` supports explicit newlines and automatic wrapping;
- `button`, `checkbox`, `radio`, and `text_input` wrap Dear ImGui's native
  controls instead of reimplementing interaction or rendering;
- `native` gives arbitrary ImGui content (ImPlot, ImNodes, ImSpinner, custom draw
  lists) an explicit rectangle, stable ID, and clip;
- `theme` and per-widget `.style()` override backgrounds, borders, text colors,
  and padding; missing values fall back to the active `ImGuiStyle`.

All components support fixed width/height, fit-content, and stretch independently
on each axis. The tree is immediate-mode and per-frame: persistent state (text
buffers, checked values, selected radio values) is supplied by the application.

```cpp
#include "cyberverse/ui/widgets.hpp"

void settings_panel::draw(cyberverse::ui::panel_context&) {
  static std::string name;
  static bool remember = true;

  ui::widgets::flexbox root;
  root.padding({ 20, 16, 20, 16 }).gap(8);

  auto& title = root.add(ui::widgets::flexbox{}
                             .direction(ui::widgets::flex_direction::row)
                             .gap(8));
  title.add(ui::widgets::text{ "Settings" });
  root.add(ui::widgets::text_input{ "player.name", name }
               .placeholder("Name")
               .width_stretch());
  root.add(ui::widgets::checkbox{ "Remember", remember });
  root.add(ui::widgets::button{ "Apply", [] { /* ... */ } }.width_stretch());

  ui::widgets::render(root);
}
```

### Why not ImGui tables?

`ImGui::BeginTable()` / `TableSetupColumn()` are excellent for tabular data,
forms, and grids, but they are not a general flexbox: tables have no vertical
grow/stretch, no per-row flex weights, no per-item min/max, and no layout pass
independent of drawing. `ui::widgets` therefore uses Yoga for flexbox and leaves
native tables available to applications. A dedicated `widgets::table` wrapper
may be added later; it will not replace the flexbox engine.

### IME

`text_input` uses Dear ImGui's committed UTF-8 text path, so CJK input works
through the platform text callback. On non-Windows platforms `ui::system`
installs ImGui's `Platform_SetImeDataFn` and forwards the caret rectangle to
`platform::window::set_ime_caret()`; on Windows the native ImGui IME path uses
`PlatformHandleRaw`. Inline preedit text and candidate-window positioning on
macOS are not implemented yet (see `docs/roadmap.md`, M5).

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

## widgets2 (experimental)

`ui::widgets2` is a separate static target,
`cyberverse::ui::widgets2_static_lib`. It does not use Yoga or the old
`widgets` tree. It is a small, functional design for fixed UI:

- every persistent component value lives in a caller-owned `widgets2::state`;
- `component<Impl>` is a static wrapper, not a polymorphic base;
- `impl` types are hidden behind factory functions;
- child components are passed as named lvalues and stored as references in
  `stack<Ts...>`; there is no `std::vector` and no ownership transfer;
- factories return `component<Impl>` by value, so all structures stay on the
  caller's stack;
- the only `std::function` use is the persistent event handler in `state`.

```cpp
#include "cyberverse/ui/widgets2.hpp"

widgets2::state row_s{"row"};
widgets2::state title_s{"title"};
widgets2::state ok_s{"ok"};
widgets2::theme appearance;
widgets2::animator animations;

ok_s.on_event([](widgets2::state& s, widgets2::event_type event) {
  if (event == widgets2::event_type::click) {
    s.set_visibility(widgets2::visibility::invisible);
  }
});

auto title = widgets2::text(title_s, "Settings");
auto ok = widgets2::button(ok_s, "OK");

auto row = widgets2::stack(
    row_s,
    {.direction = widgets2::stack_direction::horizontal, .gap = 8.0f},
    title,
    ok);

widgets2::run(row, {dt, appearance, animations});
```

`run()` is the recommended fixed frame entry: it ticks the component tree,
measures, assigns bounds, draws, and advances the ImGui cursor. `dt` belongs to
the update phase; the draw context does not carry time or the animator.

Visibility is stored in state: `visible`, `invisible` (layout retained, not
drawn), and `collapsed` (neither layout nor draw). `opacity` and `font_size` are
animatable float properties; `font_size` changes text/button measurement and
layout.

Factories accept optional full style overrides:

```cpp
widgets2::button(ok_s, "Delete", widgets2::button_style{
    .background = widgets2::color{0.8f, 0.2f, 0.2f, 1.0f},
});
```

An override replaces the global theme style for that component; it is not a
field-level merge. Without an override, `run()`'s theme is used.

The target depends on Dear ImGui and `cyberverse::animation::static_lib` (for
easing helpers). Only `run.hpp` and the implementation `.cpp` files include
ImGui; component configuration headers stay ImGui-free. Dynamic child lists,
wrapping, scrolling, and clipping are not part of this first version.

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

Yoga is fetched from `https://github.com/facebook/yoga.git` at tag `v3.2.1`
unless an existing `yogacore` target is available. To use a local checkout, pass
`-DCYBERVERSE_UI_YOGA_DIR=/path/to/yoga`; the configure step builds only Yoga's
`yoga/` subdirectory. See [THIRD_PARTY_NOTICES.md](THIRD_PARTY_NOTICES.md).

The default suite does not open a window or require Vulkan/MoltenVK. The real
GPU smoke test is opt-in:

```bash
CYBERVERSE_UI_TEST_GPU=1 /tmp/cyberverse-ui-test/test/cyberverse_ui_tests "[gpu]"
```
