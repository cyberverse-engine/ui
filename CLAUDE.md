# cyberverse-ui rules

`cyberverse::ui` is the Cyberverse Engine **layered UI runtime**. It hard-depends
on Dear ImGui and DiligentEngine. It is not a backend-agnostic framework, not a
widget library, and it does not provide a layout engine.

## Scope

- Keep the fixed dependency set: `cyberverse::runtime::static_lib`,
  `cyberverse::platform::static_lib`,
  `cyberverse::render_core::static_lib`, and `Diligent-Imgui` (Dear ImGui).
- There is no `ui::renderer`/backend interface, no backend selection option, and
  no fallback to another UI or rendering backend. Configure-time failure is the
  intended behavior when `Diligent-Imgui` is unavailable.
- Do not add a layout engine (flex, anchors, stacks), a widget/component library,
  HUD/menu/diagnostics types, or layer-level modal/capture policies. HUD and
  menu are application concepts.
- Do not add font files, assets, or application types to the library.

## API rules

- `ui::system` owns the single Dear ImGui context, the Diligent ImGui renderer,
  the input bridge, fonts, theme, and the layer stack. A non-headless frame
  binds the frame's color/depth targets and resets the scissor before the
  overlay pass.
- One engine frame opens at most one ImGui frame. `system::begin_frame()` /
  `draw()` / `end_frame()` are paired; a failed begin means no draw/end.
- `ui::layer` is a UI layer: name, order, visibility, and an ordered panel
  list. It is not a `runtime::frame_stage`; `ui::frame_stage` is the only
  runtime adapter.
- Every panel is a managed Dear ImGui window that fills the layer's logical
  viewport. Panels must draw content only and position it with native Dear ImGui
  APIs (`SetCursorPos`, `Dummy`, `Spacing`, `BeginChild`, draw lists, widgets).
- Layer and panel structure changes are only allowed between frames; throw
  `std::logic_error` during an active frame. `set_order`/`set_visible` follow the
  same rule.
- If a panel throws, its window must still end, the ImGui frame must be
  discarded, and the exception must propagate. The next frame must be usable.
- Keep `cyberverse/ui/imgui.hpp` as the only public header that includes
  `imgui.h`. Core headers (`system.hpp`, `layer.hpp`, `panel*.hpp`,
  `frame_stage.hpp`, `font.hpp`, `theme.hpp`) stay free of ImGui and Diligent
  includes.
- Single-threaded, render-thread only. Do not add hidden threads or
  asynchronous UI frames.
- One process, one ImGui context, one `ui::frame_stage`.

## Layer z-order

`detail/z_order.cpp` is the only file that includes `imgui_internal.h`. If an
upgrade removes `BringWindowToDisplayFront`/`FindWindowByName`, drop the
promotion and document the degraded overlapping-layer guarantee instead of
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

## Dependency direction

`ui` -> `runtime`, `platform`, `render-core`, Dear ImGui, DiligentEngine.
Applications depend on `ui`; `ui` never depends on application code or the
sandbox demo.
