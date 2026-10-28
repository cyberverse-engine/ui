# cyberverse-ui rules

`cyberverse::ui` is the Cyberverse Engine **layered UI runtime** with an optional
standard widget layer. It hard-depends on Dear ImGui and DiligentEngine and
privately links Yoga for flexbox layout. It is not a backend-agnostic framework.

## Scope

- Keep the fixed dependency set: `cyberverse::runtime::static_lib`,
  `cyberverse::platform::static_lib`,
  `cyberverse::render_core::static_lib`, `Diligent-Imgui` (Dear ImGui), and the
  private `yogacore` target.
- There is no `ui::renderer`/backend interface, no backend selection option, and
  no fallback to another UI or rendering backend. Configure-time failure is the
  intended behavior when `Diligent-Imgui` is unavailable.
- `ui::widgets` is the only standard component layer. It must remain a thin
  adapter over Dear ImGui native controls; do not reimplement button, checkbox,
  radio, text input, scrolling, or text editing.
- Flexbox layout comes from Yoga. Do not add anchors, stacks, grid, flex-wrap, or
  a second layout engine. ImGui tables are not the flexbox backend.
- Do not add HUD/menu/diagnostics types or layer-level modal/capture policies;
  HUD and menu are application concepts.
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
  viewport. Panels must draw content only; they may use native Dear ImGui APIs
  or `ui::widgets::render(root)`.
- Layer and panel structure changes are only allowed between frames; throw
  `std::logic_error` during an active frame. `set_order`/`set_visible` follow the
  same rule.
- If a panel throws, its window must still end, the ImGui frame must be
  discarded, and the exception must propagate. The next frame must be usable.
- Keep `cyberverse/ui/imgui.hpp` as the only **entry point** header that exposes
  `imgui.h`. Component headers under `cyberverse/ui/widgets/` include only
  `theme.hpp`, `panel.hpp`, and standard-library headers; never ImGui, Diligent,
  or Yoga. Layout internals live in `widgets/detail/` and may include Yoga/ImGui.
- `ui::system` installs ImGui's `Platform_SetImeDataFn` on non-Windows platforms
  and forwards caret data to `platform::window::set_ime_caret()`; keep Windows on
  ImGui's native IME path. `text_input` must use ImGui's committed UTF-8 text
  path, not a custom text editor.
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
- Yoga is fetched at tag `v3.2.1` (`SOURCE_SUBDIR yoga`) or provided through
  `CYBERVERSE_UI_YOGA_DIR` / an existing `yogacore` target. Keep it PRIVATE and
  keep its license notice in `THIRD_PARTY_NOTICES.md`.
- Use fake/headless hosts for unit tests; the default suite must not require a
  GPU or window. The real GPU smoke test is opt-in through
  `CYBERVERSE_UI_TEST_GPU=1`.
- Format with the repository `.clang-format` (120 columns, 2-space indent).
- README, API docs, and code comments are English.

## Dependency direction

`ui` -> `runtime`, `platform`, `render-core`, Dear ImGui, DiligentEngine, and
Yoga (private implementation detail). Applications depend on `ui`; `ui` never
depends on application code or the sandbox demo.
