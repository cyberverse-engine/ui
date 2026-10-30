# ui

Layered UI runtime for Cyberverse Engine applications. `cyberverse::ui` owns the
single Dear ImGui context, the Diligent ImGui renderer, the input bridge, fonts,
theme, and an ordered stack of UI windows.

This is v0.5. The library hard-depends on Dear ImGui and DiligentEngine. There is
no UI backend interface and no renderer abstraction. Every `ui::window` is a
managed ImGui window; by default it fills the system viewport, and a window may
request a local rectangle for sidebars or other non-fullscreen content. Window
content may use native Dear ImGui directly or the `ui::widgets` component layer.

## Dependencies

- `cyberverse::runtime::static_lib`
- `cyberverse::platform::static_lib`
- `cyberverse::render_core::static_lib`
- `Diligent-Imgui` (Dear ImGui, shipped through DiligentTools)
- `cyberverse::animation::static_lib` for widget property animation

`Diligent-Imgui` is a hard requirement. If DiligentEngine is configured without
`DILIGENT_BUILD_TOOLS=ON`, configuration fails with an actionable error instead
of falling back to another backend. Configure with
`-DCYBERVERSE_DILIGENT_DIR=<DiligentEngine source tree>` when the sibling
`../DiligentEngine` checkout is not present.

Yoga and the legacy immediate-mode widget tree are no longer dependencies.

## Contents

- `ui::host`: owns the ImGui context, Diligent renderer, fonts, theme, input
  bridge, and ordered window list. One engine frame opens at most one ImGui
  frame.
- `ui::window` / `ui::window_context`: one managed top-level window, its draw
  order/visibility, and the frame information passed to `draw()`.
- `ui::rect` / `ui::layout_size`: shared geometry types. Windows and widgets use
  the same `ui::rect`; `layout_size` is the width/height pair returned by
  measurement and used by layout.
- `ui::frame_stage`: the only `runtime::frame_stage` adapter. It draws every
  window in one overlay pass and should be registered after all 3D stages.
- `ui::widgets`: the functional component layer. Components are created by
  factories over caller-owned `widgets::state`; containers store references to
  named children. There is no virtual widget base and no `std::vector` inside
  widgets.
- `cyberverse/ui/imgui.hpp`: the explicit Dear ImGui entry point for window
  implementations and style/escape-hatch code.

## Windows

A window is a content unit managed by the host. The host owns the Dear ImGui
window; the application only paints content and supplies one stable id:

```cpp
#include "cyberverse/ui.hpp"
#include "cyberverse/ui/imgui.hpp"

class status_window final : public cyberverse::ui::window {
public:
  status_window() : window("app.status") {}

  [[nodiscard]] cyberverse::ui::window_feature features() const noexcept override {
    return cyberverse::ui::window_feature::no_input;
  }
  void draw(cyberverse::ui::window_context& context) override {
    ImGui::SetCursorPos(ImVec2{ 16.0f, 16.0f });
    ImGui::TextUnformatted("Status");
    (void)context;
  }
};
```

The managed window fills the host viewport by default. A window may override
`window_rect(viewport)` to request a local rectangle; the host clamps it and
reports it through `window_context::bounds`, whose type is the shared `ui::rect`.
Feature flags include scrollbar, transparent background, no input, focus on
appearing, and bring-to-front on focus. `window::order()` controls draw order;
equal orders keep insertion order. The constructor stores the id as a
`std::string`, so its `data()` is null-terminated and can be passed directly to
Dear ImGui.

## Widgets

The widgets API is deliberately different from the legacy per-frame tree:

- persistent component values live in caller-owned `widgets::state`;
- factories return a static `component<Impl>` wrapper;
- containers accept named lvalue children and store references;
- `std::function` is used only by `state::on_event`;
- `widgets::run(window_context, theme, root, opacity = 1.0f)` is the fixed
  frame entry;
- widget `measure()` returns `ui::layout_size`;
- `widgets::rect` and `widgets::layout_size` are aliases of the shared `ui`
  types, not separate geometry types.

```cpp
#include "cyberverse/ui/widgets.hpp"

widgets::state row_s{ "row" };
widgets::state title_s{ "title" };
widgets::state ok_s{ "ok" };
widgets::theme appearance;

ok_s.on_event([](widgets::state& s, widgets::event_type event) {
  if (event == widgets::event_type::click) {
    s.set_visibility(widgets::visibility::invisible);
  }
});

auto title = widgets::text(title_s, "Settings");
auto ok = widgets::button(ok_s, "OK");
auto row = widgets::stack(
    row_s,
    {.direction = widgets::stack_direction::horizontal, .gap = 8.0f},
    title,
    ok);

widgets::run(window, appearance, row);
```

`run()` ticks the component tree, measures, assigns bounds, draws, and advances
the ImGui cursor. Animation time comes from `window.delta_time`; a zero delta
freezes all animations. The optional fourth argument is the initial inherited
opacity for the whole root subtree. Widgets have no separate pause/speed
control; pause or time-scale the runtime clock instead.

### Components

- `text`, `button`;
- `checkbox`, `radio` with caller-owned bool/int values;
- `text_input` with caller-owned `std::string`;
- `divider` (horizontal/vertical);
- `stack`, no-wrap `flexbox`, and `scroll_view` with simultaneous vertical and
  horizontal scrolling;
- `box`, a styled single-child container replacing the old `widgets::panel`.

Visibility is stored in state: `visible`, `invisible` (layout retained, not
drawn), and `collapsed` (neither layout nor draw). `opacity`, `font_size`,
`width`, and `height` are animatable float properties. Flexbox reads
`flex_grow`, `flex_shrink`, and `flex_basis` from the child state. Layout results
are recomputed from animated inputs; there is no separate layout-result FLIP
layer.

If an explicit width or height is zero, measurement and drawing skip the
component's children. A container whose bounds become zero also skips child
layout and drawing. Children with zero explicit dimensions are skipped by
container layout.

Scroll offsets are stored in `state.scroll_x` / `state.scroll_y`. Scroll views
use a compile-time child tuple, clip content to their viewport, and overlay
draggable horizontal/vertical scrollbars. Wheel input supports both axes;
Shift+wheel maps vertical wheel input to horizontal scrolling.

Factories accept optional full style overrides:

```cpp
widgets::button(ok_s, "Delete", widgets::button_style{
    .background = widgets::color{0.8f, 0.2f, 0.2f, 1.0f},
});
```

An override replaces the global theme style for that component; it is not a
field-level merge. Without an override, `run()`'s theme is used.

## Frame lifecycle

1. `ui::frame_stage::on_render` checks `host::has_visible_content()`.
2. It obtains a `render_core::frame_context` (normally
   `render_device::begin_frame`).
3. `host::begin_frame(frame, dt)` binds the frame's color/depth targets,
   resets the scissor, calls the Diligent renderer's `NewFrame`, feeds platform
   input into ImGui, and calls `ImGui::NewFrame()`.
4. `host::draw()` visits visible windows from the lowest `order()` to the
   highest, creates one managed ImGui window per visible window (full viewport
   by default, or `window::window_rect()` when supplied), and calls
   `window::draw(context)`.
5. `host::end_frame()` calls `ImGui::Render()`, submits one
   `RenderDrawData()` pass, resets the scissor, and records
   `WantCaptureMouse` / `WantCaptureKeyboard`.

`begin_frame()` returning false means no drawable frame; do not call `draw()` or
`end_frame()`. `end_frame()` without an active frame is a no-op. Window structure may only
change between frames; mutating it during a frame throws `std::logic_error`. A
window exception ends its window and discards the ImGui frame before
propagating.

`wants_mouse()` / `wants_keyboard()` reflect the most recently completed frame,
so applications see one frame of latency. Hide windows or use `no_input`
windows when input must be blocked immediately.

## Ownership and threading

- `ui::host` must be destroyed before the render device and platform window.
  Remove the `ui::frame_stage` from the runtime before destroying the system.
- One process has one Dear ImGui context. Applications must not link another
  ImGui copy or create a second context.
- All calls are render-thread calls. There are no hidden worker threads and no
  asynchronous UI frames.
- Register exactly one `ui::frame_stage`; it must render after 3D and before
  present.
- Widget `state` objects are non-copyable/non-movable and must outlive the
  component tree that references them.

## Window z-order

Windows are drawn from the lowest `order()` to the highest, and ties keep
insertion order. The implementation raises windows in draw order through
`imgui_internal.h`, isolated in `src/cyberverse/ui/detail/z_order.cpp`. If a
Dear ImGui upgrade removes that internal API, overlapping windows no longer get
a strict z-order guarantee; applications should then avoid overlapping
interactive windows and control input through visibility and `no_input`.

## Headless mode

`host_config::headless = true` creates no GPU resources. It builds the font
atlas eagerly with a test-only texture identifier and still produces Dear ImGui
draw data, so window ordering, input mapping, and exception cleanup can
be tested without a GPU or window. A headless system may own a platform headless
window to inject deterministic input.

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
