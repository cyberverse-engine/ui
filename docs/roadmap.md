# UI 路线图

- **状态：** v0.4：M0–M4 已实现；M5–M6 待做
- **最后更新：** 2026-10-28
- **本版重点：** 固定 ImGui + Diligent 分层运行时；基于 ImGui 原生控件和 Yoga 的标准组件库
- **范围：** [`cyberverse-engine/ui`](../README.md)
- **硬依赖：** `runtime`、`render-core`、`platform`、Dear ImGui、DiligentEngine；实现私有依赖 Yoga
- **相关文档：** [runtime 路线图](../../runtime/docs/roadmap.md)、[render-core 路线图](../../render-core/docs/roadmap.md)、[sandbox-demo](../../sandbox-demo/README.md)

本文档定义 `cyberverse::ui` 的目标架构、边界和分阶段路线图。

> **实现状态（2026-10-28）：** M0–M3 的分层运行时已经落地；M4 的标准组件库
> （`ui::widgets`）也已实现并通过布局、主题、交互、CJK 输入和无 GPU 测试。
> 组件库是 ImGui 原生控件之上的**薄层**：布局、尺寸、主题和组合由 `ui` 负责，
> button/checkbox/radio/text input/scroll child 不重新实现。

---

## 1. 结论摘要

`ui` 是 **Dear ImGui + DiligentEngine 的分层 UI 运行时 + 基础组件库**：

- `ui` 直接链接 Dear ImGui 和 DiligentEngine，不提供后端接口、后端选择或替代实现；
- `ui::system` 拥有唯一的 ImGui 上下文、Diligent ImGui 渲染器、输入桥、字体、主题和层栈；
- 一个引擎帧只开启一次 ImGui 帧，所有层共享一次 `ImGui::NewFrame()` / `ImGui::Render()`；
- `ui::layer` 表示一层 UI，包含名称、顺序、可见性和 panel 列表；
- `ui::panel` 是应用内容单元，受管窗口固定填满层 viewport；内容可以用原生 ImGui，
  也可以用 `ui::widgets`；
- `ui::widgets` 提供 flexbox、panel、scroll_view、collapse_group、text、button、
  checkbox、radio、text_input、native；
- 布局由 **Yoga v3.2.1**（MIT）完成，只在 `ui` 静态库内部链接，不出现在公共头；
- 控件外观优先使用 ImGui 原生绘制，组件层只做尺寸、裁剪、主题和组合；
- `native` 为 ImPlot、ImNodes、ImSpinner 等第三方领域库提供显式矩形和 ID；
- `ui::frame_stage` 是唯一的 `runtime::frame_stage` 适配器，一次绘制全部层。

---

## 2. 架构与依赖

```text
应用 / sandbox-demo
  ├─ 世界渲染（render-core）
  └─ ui::panel / ui::widgets
        │
        ▼
┌───────────────────────────────────────────────┐
│ ui::system                                    │
│  ├─ layer stack（order / visible / panels）   │
│  ├─ input bridge（platform → ImGui）           │
│  ├─ font / theme / scale / IME caret hook      │
│  ├─ ImGui context + Diligent ImGui renderer    │
│  └─ ui::frame_stage                            │
└───────────────────────────────────────────────┘
```

依赖方向：

```text
runtime ─────────┐
render-core ─────┼──→ ui ──→ application
platform ────────┘
Dear ImGui ──────┘
DiligentEngine ──┘
Yoga（PRIVATE）──┘
```

- `render-core` 拥有 Diligent device、swap chain 和 frame context；`ui` 不创建设备。
- `ui` 直接依赖 `platform`，因为输入、窗口逻辑尺寸、framebuffer scale、光标和 IME 光标
  都需要平台窗口。
- `ui` 直接依赖 Dear ImGui 和 DiligentEngine，没有 `ui::renderer` 接口。
- Yoga 只由 `cyberverse_ui_static_lib` 私有链接；公共组件头不包含 `<yoga/Yoga.h>`。
- 一个进程只有一份 ImGui 代码和上下文；应用不得再链接另一份 ImGui 或创建第二个上下文。
- 单线程、render thread only；没有隐藏的布局线程、字体线程或异步 UI 帧。

### 2.1 为什么用 Yoga 而不是 ImGui Table

`ImGui::BeginTable` / `TableSetupColumn` 适合表格、表单和网格，但不能承担通用 flexbox：

| 能力 | Table/Column | 本需求 |
| --- | --- | --- |
| 横向固定宽、比例拉伸 | 支持 | 需要 |
| fit-content 列宽 | 依赖内容绘制后确定 | 需要布局前测量 |
| 行高 fit-content | 支持 | 需要 |
| 行高 stretch / grow | 无 | 需要 |
| 每子项 min/max | 无 | 需要 |
| 两轴独立 stretch | 仅列宽 | 需要 |
| 任意嵌套组件树 | 列 schema 全表共享，需要嵌套 table | 需要 |
| 布局与绘制解耦、纯布局测试 | 强耦合 | 需要 |

因此 `ui::widgets::flexbox` 使用 Yoga；Table API 不进入布局引擎。应用仍然可以直接使用
原生 `BeginTable`，将来如有重复需求可增加独立的 `widgets::table` 薄包装（不在当前范围）。

---

## 3. 核心对象

### 3.1 `ui::system`

拥有唯一的 ImGui context、Diligent renderer、输入桥、字体图集、主题、缩放和层栈。

- `begin_frame()` / `draw()` / `end_frame()` 配对；`begin_frame()` 返回 false 时不得 draw/end。
- 非 headless 帧会绑定 color/depth target、重置 scissor，再进入 ImGui 帧。
- 输入桥把 platform 的鼠标、按键、文本、滚轮转成 ImGui IO；IME 提交文本走同一路径。
- 非 Windows 平台安装 `Platform_SetImeDataFn`，把 ImGui 计算的光标矩形转发到
  `platform::window::set_ime_caret()`；Windows 使用 ImGui 默认 IME 实现。
- `wants_mouse()` / `wants_keyboard()` 反映上一已完成帧的全局捕获。
- 层和 panel 结构只在帧间修改；帧中修改抛 `std::logic_error`。

### 3.2 `ui::layer`

- 名称、顺序、可见性、panel 列表；不是 runtime stage。
- 层之间按 order 稳定排序，同 order 保持插入顺序。
- 层不提供 modal/capture 策略；应用通过可见性、`panel_feature::no_input` 和
  `wants_mouse()` / `wants_keyboard()` 管理输入。

### 3.3 `ui::panel`

- 每个 panel 对应一个受管、全 viewport 的 ImGui window；panel 不创建顶层窗口。
- `draw(panel_context&)` 绘制内容；可以用原生 ImGui，也可以调用 `ui::widgets::render()`。
- `panel_feature::transparent` / `no_input` 保留；异常结束时窗口必须关闭并丢弃 ImGui 帧。

### 3.4 `ui::frame_stage`

- 唯一的 runtime 适配器；在一次 overlay pass 中绘制所有层。
- 不拥有 ImGui 资源；必须在 3D stage 之后、present 之前。

---

## 4. `ui::widgets`

### 4.1 组件清单

| 组件 | 实现方式 | 说明 |
| --- | --- | --- |
| `flexbox` | Yoga | row/column 及 reverse、gap、padding、justify、align、grow/shrink |
| `panel` | flexbox + draw list | 背景、实线边框、圆角、裁剪子项 |
| `scroll_view` | flexbox + `ImGui::BeginChild` | 原生滚动/裁剪，内部列布局 |
| `collapse_group` | 轻量 header + 内部 flexbox | 应用持有 open 状态 |
| `stack` | flexbox 薄封装 | 水平/垂直不换行，两轴默认 fit-content |
| `divider` | draw list 细线 | 水平/垂直，固定厚度，交叉轴 stretch |
| `text` | `ImGui::CalcTextSize` / draw list | 多行、自动换行、裁剪 |
| `button` | `ImGui::Button` | 原生按钮 |
| `checkbox` | `ImGui::Checkbox` | 外部 `bool` 状态 |
| `radio` | `ImGui::RadioButton` | 外部 `int` 选择值 |
| `text_input` | `ImGui::InputTextWithHint` + `imgui_stdlib` | 单行、placeholder、password、read-only |
| `native` | 显式 rect 回调 | ImPlot/ImNodes/ImSpinner/自定义 draw list |

v1 **不实现**：flex-wrap、grid、dock、绝对定位、浮动 panel、动画组件、表格组件。

### 4.2 尺寸模型

每个组件都支持三种宽/高策略，且可分别设置：

| 模式 | 主轴 | 交叉轴 |
| --- | --- | --- |
| `fixed(v)` | 精确 v | 精确 v |
| `fit_content` | 由内容/Yoga 测量 | 由内容测量 |
| `stretch` | `flex-grow=1`、`flex-basis=0` | `align-self: stretch` |

- 根组件在 `render()` 中把当前 ImGui content region 作为 containing block。
- 支持 `min_width/max_width/min_height/max_height`、`margin`、显式 `grow/shrink`。
- 尺寸为逻辑点；DPI 由 ImGui font/style 统一缩放。
- 负值、NaN 会被清洗；`min > max` 会提升上限。

### 4.3 公共 API 示例

```cpp
#include "cyberverse/ui/widgets.hpp"

void panel::draw(ui::panel_context&) {
  bool remember = true;
  std::string name;

  ui::widgets::flexbox root;
  root.padding({ 20, 16, 20, 16 }).gap(8);

  ui::widgets::panel card;
  card.width_fit_content().padding({ 24, 20, 24, 20 }).gap(10);
  auto& content = root.add(std::move(card));

  auto& title = content.add(ui::widgets::flexbox{}.direction(ui::widgets::flex_direction::row).gap(8));
  title.add(ui::widgets::text{ "Settings" });
  content.add(ui::widgets::text_input{ "player.name", name }.placeholder("Name").width_stretch());
  content.add(ui::widgets::checkbox{ "Remember", remember });
  content.add(ui::widgets::button{ "Apply", [] { /* ... */ } }.width_stretch());

  ui::widgets::render(root);
}
```

### 4.4 主题

- `ui::widgets::theme` 按组件类型提供可选的背景、hover/active、文本、禁用文本、
  实线边框、圆角、focus ring 和 padding。
- 组件级 `.style(widget_style)` 优先于 `theme`；未设置的字段回退到当前 `ImGuiStyle`。
- 所以 `ui::system` 的 dark/light/custom ImGui style 仍然是最终外观基线。
- 容器背景/边框由 draw list 绘制；button/checkbox/radio/input 通过 push/pop ImGui style
  保持原生控件行为。

### 4.5 `native`

```cpp
content.add(ui::widgets::native{
  "plot",
  [](const ui::rect& bounds) {
    if (ImPlot::BeginPlot("##plot", ImVec2(bounds.width, bounds.height))) { /* ... */ }
    ImPlot::EndPlot();
  },
  [] { return ui::widgets::content_size{ 320, 180 }; } });
```

- 回调在 ImGui 帧内、当前 window 内运行，cursor 已定位到 rect，并处于裁剪区域。
- 第三方 context 由应用创建/销毁；`ui` 不链接 ImPlot/ImNodes/ImSpinner。
- 如果没有 measure 回调，`native` 填充可用约束；需要 fit-content 时提供显式尺寸。

### 4.6 IME

已实现：

- platform 的 `ime_caret` 接口和 headless 存储/测试；
- 非 Windows 平台由 `ui::system` 安装 ImGui `Platform_SetImeDataFn`，把
  `InputPos`/`InputLineHeight`/`WantVisible` 转发给平台窗口；
- `text_input` 使用 ImGui 的提交字符串路径，CJK 提交文本可正常写入；
- Windows 设置 `PlatformHandleRaw`，走 ImGui 默认 IME。

M5 待做：

- macOS 内联 preedit 文本、候选窗口定位和 GLFW `NSTextInputClient` 的候选/选中范围桥接；
- 组合输入期间的视觉反馈（preedit 下划线/候选列表）；
- 更完整的平台 IME 测试（真实窗口 opt-in）。

---

### 4.7 `widgets2`（实验性）

`widgets2` 是独立的 `cyberverse::ui::widgets2_static_lib` 目标，不依赖旧
`widgets`、Yoga、runtime、platform 或 render-core；它只链接 Dear ImGui 和
`animation`，用于验证新的函数式/静态组件设计。

设计要点：

- `state` 是调用者持有的持久状态：bounds、width/height/opacity/font_size、
  flex_grow/flex_shrink/flex_basis、scroll_x/scroll_y 属性、动画元数据和
  visibility；不保存 animator 或子组件。
- 组件由工厂创建：`text`、`button`、`checkbox`、`radio`、`text_input`、
  `divider`、`stack`、`flexbox`、`scroll_view`，返回 `component<Impl>`；不公开
  组件实现类型。
- `component<Impl>` 是静态包装，默认处理 visibility、opacity、measure、
  draw 和递归 tick；没有虚函数、类型擦除或 `std::vector`。
- 容器模板只保存子组件引用，子项必须是有名字的左值；临时对象不能作为子项。
- 每帧推荐入口为 `run(root, frame_context{dt, theme, animator})`，固定顺序：
  tick → measure → 设置 bounds → draw。dt 只属于更新阶段，draw_context 不包含
  dt/animator。
- 采用方案 A：布局结果由动画后的 width/height/flex_* 输入重新计算，不做
  target/visual 双 bounds 或 FLIP。
- `state` 的 `std::function` 事件回调是明确的分配例外；button 点击发出
  `click`，checkbox/radio/text_input 变化发出 `value_changed`。
- `visibility` 三态：visible、invisible（布局保留、不绘制）、collapsed
  （不布局、不绘制）；容器跳过 collapsed 和零尺寸子项。
- 显式 width 或 height 为 0 时，组件跳过子项 measure/draw；bounds 任一轴为 0
  时也跳过绘制。该规则适用于 stack/flexbox/scroll_view。
- `stack` 是固定子项的一维排布；`flexbox` 支持 row/column、gap/padding、
  justify、align、grow/shrink/basis，不支持 wrap。
- `scroll_view` 同时支持横向和纵向滚动，滚动偏移存放在 state 的 scroll_x/
  scroll_y；支持滚轮、Shift+滚轮、双滚动条和拖动。
- `checkbox`/`radio` 使用外部 bool/int；`text_input` 使用外部 std::string；
  三者都使用 ImGui 原生控件。
- text/button/checkbox/radio/text_input/divider/scroll_view 工厂接受可选完整
  style override；未传入时使用 `run()` 的全局 theme。override 是整体替换，
  不做字段级 merge。
- v1 只支持编译期固定子项；动态列表、flex-wrap、文本换行、虚拟滚动和
  collapse_group 后续再做。

## 5. 帧流程

1. `ui::frame_stage::on_render` 检查 `has_visible_content()`。
2. 取得 `render_core::frame_context`。
3. `system::begin_frame(frame, dt)` 绑定目标、重置 scissor、提交输入、`ImGui::NewFrame()`。
4. `system::draw()` 按 order 遍历可见层，为每个可见 panel 创建受管窗口并调用 `draw()`。
5. `panel::draw()` 可以用原生 ImGui，或调用 `widgets::render(root)`；后者完成 Yoga 布局、
   绘制、交互，并提交根矩形作为 ImGui item。
6. `system::end_frame()` 调用 `ImGui::Render()`，提交一次 `RenderDrawData()`，
   记录全局捕获。

`begin_frame()` 返回 false 时不调用 draw/end。`end_frame()` 没有活动帧时是 no-op。

---

## 6. 里程碑

### M0–M3（已完成）

- 固定依赖、ImGui + Diligent 生命周期、输入桥；
- `ui::layer` / `ui::panel` / `ui::frame_stage`；
- 层内 panel 管理和 z-order；
- sandbox HUD/menu 迁移到受管 panel；
- 测试：系统、字体、输入桥、层、panel、frame stage、z-order。

### M4 — 标准组件库（已完成，2026-10-28）

- Yoga v3.2.1 依赖和私有链接；
- `ui::widgets` 基础类型、尺寸模型、主题解析、Yoga 布局引擎；
- flexbox、panel、scroll_view、collapse_group、stack、divider、text、button、
  checkbox、radio、text_input、native；
- IME 光标钩子（platform + ImGui callback）；
- 测试：固定/fit/stretch、gap/padding、wrap、native 测量、主题、按钮/复选框/单选交互、
  scroll/collapse、CJK 输入、IME caret；
- sandbox HUD/menu 迁移，加入文本输入、复选框、单选和原生按钮。

### M4.1 — widgets2 实验（进行中）

- 独立 target，状态外部化，工厂 + `component<Impl>`。
- v1：theme、text、button、checkbox、radio、text_input、divider、stack、
  no-wrap flexbox、双轴 scroll_view、visibility/opacity/font_size、事件回调、
  `run()` 固定帧入口。
- 零尺寸容器跳过子项计算；布局结果采用输入动画方案 A。
- 已知限制：编译期固定子项、无动态列表、无 flex-wrap、无文本换行、无虚拟滚动、
  无 collapse_group、`std::function` 仅用于 state 事件回调。

### M5 — IME 与文本编辑完善（待做）

- macOS inline preedit/candidate 定位；
- 文本选中、剪贴板、撤销/重做的显式测试；
- GLFW 平台 IME 的 opt-in 真实窗口测试；
- 可选的密码/只读/长度限制辅助 API（基于 ImGui 原生 flags）。

### M6 — 健壮性与发布（待做）

- 布局边界/异常输入测试、主题回归测试、截图对比；
- GPU smoke 覆盖 widgets；
- 可选领域库接入示例（ImPlot/ImNodes/ImSpinner，不进入核心依赖）；
- 文档、第三方 notices、版本/兼容性策略。

---

## 7. 测试策略

- 默认 `ctest` 不打开窗口、不需要 GPU；使用 headless system 和 headless platform window。
- 布局测试覆盖固定、fit-content、stretch、gap/padding、wrap、根尺寸、滚动/折叠测量。
- 交互测试用 headless 注入鼠标 down/up、文本 commit；IME caret 从 headless window 读取。
- GPU smoke 通过 `CYBERVERSE_UI_TEST_GPU=1` 显式开启。
- 公共组件头保持不包含 ImGui/Yoga；widgets 公共头只依赖 `theme.hpp`/`panel.hpp`。
- 所有测试在 `/tmp` 构建和运行，不写仓库。

---

## 8. 非目标与限制

- 不实现完整 CSS flexbox：无 wrap、order、百分比、baseline、margin auto、grid。
- 不提供 HUD/menu/diagnostics 类型或预设；这些是应用概念。
- 不提供层级的 modal/capture 策略。
- 不做多 viewport、docking、浮动 panel、布局编辑器。
- 不重绘原生控件；主题只覆盖 ImGui 可 push 的颜色/圆角和容器绘制。
- 不把 ImPlot/ImNodes/ImSpinner/MetricsGui 作为核心依赖；MetricsGui 已停止维护，不采用。
- macOS 内联 preedit/候选定位尚未实现；当前只保证提交文本和光标矩形。
- `native` 的 fit-content 需要应用提供 measure 回调；否则填充可用空间。
- 组件树只在一帧内有效；持久状态由应用持有。

---

## 9. 第三方

| 项目 | 版本/许可 | 用途 |
| --- | --- | --- |
| [Dear ImGui](https://github.com/ocornut/imgui) | 随 DiligentTools，MIT | 原生控件、绘制、输入 |
| [DiligentEngine](https://github.com/DiligentGraphics/DiligentEngine) | 固定 tag，Apache-2.0 | 渲染器/后端 |
| [Yoga](https://github.com/facebook/yoga) | v3.2.1，MIT | flexbox 布局 |

- Yoga 通过 FetchContent 获取，或通过 `CYBERVERSE_UI_YOGA_DIR` 指向本地源码；
  `SOURCE_SUBDIR yoga` 只构建布局库，跳过其测试。
- Yoga 是 PRIVATE 依赖；公共头不暴露 Yoga 类型。
- ImPlot/ImNodes/ImSpinner 只在应用需要时通过 `native` 接入；不进入 `ui` 的链接依赖。
- MetricsGui 已 archived/停止维护，不采用。

---

## 10. 完成定义

- `ui` + sandbox 默认测试全绿；GPU smoke 可选通过。
- 所有组件支持 fixed/fit/stretch 宽高；容器正确裁剪/滚动/折叠。
- CJK 提交文本可输入，IME 光标矩形上报到平台。
- 公共 widgets 头不包含 ImGui/Yoga；Yoga 仅 PRIVATE 链接。
- sandbox HUD/menu 使用 widgets 后视觉与交互保持可用；截图归档。
- 文档准确描述 Table API 的边界、IME 限制和第三方策略。
