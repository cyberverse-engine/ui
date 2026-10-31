# cyberverse-ui rules

`cyberverse::ui` is the Cyberverse Engine **layered UI runtime** with a
functional widget layer. It hard-depends on Dear ImGui and DiligentEngine. It is
not a backend-agnostic framework.

## Scope

- Keep the fixed dependency set: `cyberverse::runtime::static_lib`,
  `cyberverse::platform::static_lib`,
  `cyberverse::render_core::static_lib`, `Diligent-Imgui` (Dear ImGui), and
  `cyberverse::animation::static_lib` (widget property animation).
- There is no `ui::renderer`/backend interface, no backend selection option, and
  no fallback to another UI or rendering backend. Configure-time failure is the
  intended behavior when `Diligent-Imgui` is unavailable.
- `ui::widgets` is the only widget layer. Components are factory-created from
  caller-owned `widgets::state`; containers hold references to named children.
  Do not add virtual widget bases, type erasure, `std::vector` child storage, or
  dynamic child ownership. The only intentional `std::function` is the state
  event handler.
- `ui::widgets` v1 includes `text`, `button`, `checkbox`, `radio`, `radio_group`, `text_input`, `native`,
  `divider`, `stack`, no-wrap `flexbox`, dual-axis `scroll_view`, and `box`.
  Flex item sizing is driven by animated width/height/flex_* inputs; do not add
  a separate layout-result (FLIP) animation layer.
- Explicit zero width or height must skip child measurement and drawing.
  Containers must skip zero-size children and gaps.
- `min_width` / `min_height` / `max_width` / `max_height` are flex sizing
  constraints; `native` wraps measurable raw ImGui draw callbacks without
  `std::function`.
- Widget animation time is `widget_context.window.delta_time`; there is no
  widget animator or separate pause/speed control.
- `ui::widgets` may depend on `ui::window_context` through `widget_context`,
  but must not depend on host implementation, runtime, platform, or render-core.
  It may depend on `Diligent-Imgui` and `cyberverse::animation::static_lib`.
- Top-level hosting is `ui::window`, owned directly by `ui::host`. A window is
  not a widget. It owns Dear ImGui window lifecycle, order, visibility, local
  rectangles, features, input capture, z-order, and exception cleanup.
- Do not add HUD/menu/diagnostics types or window-level modal/capture policies;
  HUD and menu are application concepts.
- Do not add font files, assets, or application types to the library.

## API rules

- `ui::host` owns the single Dear ImGui context, the Diligent ImGui renderer,
  the input bridge, fonts, theme, and the ordered window list. A non-headless
  frame binds the frame's color/depth targets and resets the scissor before the
  overlay pass.
- One engine frame opens at most one ImGui frame. `host::begin_frame()` /
  `draw()` / `end_frame()` are paired; a failed begin means no draw/end.
- There is no separate UI layer object. `ui::host` owns windows directly.
  Window `order()` controls draw order; equal orders keep insertion order.
  Window `visible()` replaces layer visibility.
- Every `ui::window` is a managed Dear ImGui window that fills the host
  viewport by default. `window::window_rect()` may request a local rectangle;
  the host clamps it. Windows must draw content only; they may use native
  Dear ImGui APIs or `ui::widgets::run(window, theme, root, opacity)`.
- Window structure changes are only allowed between frames; throw
  `std::logic_error` during an active frame.
- If a window throws, its ImGui window must still end, the ImGui frame must be
  discarded, and the exception must propagate. The next frame must be usable.
- Keep `cyberverse/ui/imgui.hpp` as the only **entry point** header that exposes
  `imgui.h`. Public widget/configuration headers stay ImGui-free. Only
  `widgets/run.hpp`, implementation `.cpp` files, and the internal
  `widgets/detail/font.hpp` / `scroll_canvas.cpp` bridge may include ImGui.
- Window ids are host-wide unique; each window supplies its id as a
  `std::string` at construction. The stored string is null-terminated and is
  also the Dear ImGui window name. It must not be empty, contain `###`, or
  contain an embedded null.
- `ui::rect` and `ui::layout_size` are shared public geometry types. Windows
  and widgets must use the same `ui::rect`; widgets may alias it as
  `widgets::rect`. Widget `measure()` returns `ui::layout_size`; do not add a
  second rect/size type.
- Widgets:
  - `state` owns id, bounds, visibility, properties, animation metadata, and the
    event callback; it never owns child widgets or animation policy.
  - `component<Impl>` is a static wrapper; no virtual methods.
  - `run()` order is tick → measure → set bounds → draw.
  - Explicit zero dimensions skip implementation measure; zero bounds skip draw.
  - `std::function` remains the one state event allocation point.
- `ui::host` installs ImGui's `Platform_SetImeDataFn` on non-Windows platforms
  and forwards caret data to `platform::window::set_ime_caret()`; keep Windows on
  ImGui's native IME path. `text_input` must use ImGui's committed UTF-8 text
  path, not a custom text editor.
- Single-threaded, render-thread only. Do not add hidden threads or
  asynchronous UI frames.
- One process, one ImGui context, one `ui::frame_stage`. The stage holds a
  non-owning host reference; remove it from the pipeline before destroying the
  host.

## Window z-order

`detail/z_order.cpp` is the only file that includes `imgui_internal.h`. If an
upgrade removes `BringWindowToDisplayFront`/`FindWindowByName`, drop the
promotion and document the degraded overlapping-window guarantee instead of
spreading internal API use.

## Build and test

- Build and test under `/tmp`, never inside the repository:
  `cmake -S . -B /tmp/cyberverse-ui-test -DCYBERVERSE_DILIGENT_DIR=<DiligentEngine>`.
- `Diligent-Imgui` comes from a DiligentEngine build with
  `DILIGENT_BUILD_TOOLS=ON` (render-core enables it by default). A missing target
  is a configure error, not a fallback.
- Use fake/headless hosts for unit tests; the default suite must not require a
  GPU or window. The real GPU smoke test is opt-in through
  `CYBERVERSE_UI_TEST_GPU=1`.
- Format with the repository `.clang-format` (120 columns, 2-space indent).
- README, API docs, and code comments are English.
- `docs/roadmap.md` is Chinese.

## Dependency direction

`ui` -> `runtime`, `platform`, `render-core`, `animation`, Dear ImGui, and
DiligentEngine. `ui::widgets` is a header-only/template-facing component layer
inside the same static library; it depends on `window_context` only through
`widget_context`, not on the host implementation.
Applications depend on `ui`; `ui` never depends on application code or the
sandbox demo.
