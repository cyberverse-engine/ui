# UI 路线图

- **状态：** v0.3 实现稿：M0–M3 已实现（不含 `ui::layout`/flex），M4–M6 待做
- **最后更新：** 2026-10-27
- **本版重点：** 固定 ImGui + Diligent；只做通用分层渲染；唯一的 flex 布局组件
- **范围：** [`cyberverse-engine/ui`](../README.md)
- **硬依赖：** `runtime`、`render-core`、`platform`、Dear ImGui、DiligentEngine
- **相关文档：** [runtime 路线图](../../runtime/docs/roadmap.md)、[render-core 路线图](../../render-core/docs/roadmap.md)、[sandbox-demo](../../sandbox-demo/README.md)

本文档定义 `cyberverse::ui` 的目标架构、边界和分阶段路线图。它与上一版设计的关键区别是：

1. `ui` **强制依赖 Dear ImGui 和 DiligentEngine**，不再考虑其他 UI 后端或渲染后端；
2. `ui` **只提供分层渲染**，不感知 HUD、diagnostics、menu 等应用用法；
3. 布局只保留 **一个 `ui::layout` 组件**，采用简化的 HTML flexbox 模型：`flex-direction`、
   `flex-grow` / `flex-shrink` / `flex-basis`、子项最小/最大值，以及 XY 双轴拉伸。

本文档是设计稿，不是实现承诺；API 细节会在里程碑中通过测试固定。

> **实现状态（2026-10-27）：** M0–M3 已落地，但本次不实现 `ui::layout`/flex。
> `ui::layer` 不再有根布局，改为 `add_panel()`；每个 panel 的受管窗口固定填满层
> viewport，内容、留白和遮罩全部使用 ImGui 原生组件与定位 API。flex 布局留待后续里程碑。

---

## 1. 结论摘要

`ui` v0.3 的定位是 **Dear ImGui + DiligentEngine 的分层 UI 运行时**：

- `ui` 直接链接并包含 Dear ImGui 和 DiligentEngine，不提供后端接口、后端选择或替代实现；
- `ui::system` 拥有唯一的 ImGui 上下文、Diligent ImGui 渲染器、输入桥、字体、主题和层栈；
- 一个引擎帧只开启一次 ImGui 帧，所有层共享一次 `ImGui::NewFrame()` / `ImGui::Render()`；
- `ui::layer` 表示 **一层 UI**，只包含名称、顺序、可见性和一个根 `ui::layout`；
- `ui::layout` 是唯一的布局组件，支持 flex 方向、伸缩比例、初始尺寸、子项 min/max 和双轴拉伸；
- 所有 panel 都通过 layout 获得显式矩形；`ui` 不做内容自适应尺寸；
- `ui` 不提供 HUD、diagnostics、menu 的类型、枚举、预设或诊断面板；这些完全由应用定义；
- `ui` 不提供层级的 modal/capture 输入策略；应用通过层可见性、panel 的 `no_input` 和全局
  `wants_mouse()` / `wants_keyboard()` 自行管理交互；
- `ui::frame_stage` 是唯一的 `runtime::frame_stage` 适配器，一次绘制全部层。

> 这是破坏性调整。当前 `ui::host`、单 panel 的 `ui::frame_stage`、anchor/stack 布局和
> “每个 UI 层各自开启一次后端帧”的模型都会被替换。迁移策略见第 10 节。

---

## 2. 本次修订的边界

| 主题 | 本版决定 |
| --- | --- |
| UI 后端 | 固定 Dear ImGui；不抽象、不替换。 |
| 渲染后端 | 固定 DiligentEngine；通过 `render-core` 使用 device/context，并直接使用 `Diligent::ImGuiDiligentRenderer`。 |
| 层语义 | 只有 `order` 和 `visible`；不提供 HUD/diagnostics/menu 预设。 |
| 层输入策略 | 不提供 `none/capture/modal`；应用自行隐藏层、设置 panel `no_input` 或阻止游戏输入。 |
| 布局 | 唯一的 `ui::layout`，简化 flexbox；删除 anchor、stack、fill、auto-size。 |
| 尺寸 | 布局总是计算显式矩形；panel 不再 `AlwaysAutoResize`。 |
| 诊断界面 | 不出现在库内；应用可自行实现任意面板。 |
| 后端替换 | 明确不支持；其他 UI/渲染后端不属于本库范围。 |

---

## 3. 目标与非目标

### 3.1 目标

- **固定依赖。** `ui` 直接依赖 Dear ImGui 和 DiligentEngine；CMake 找不到就失败，不提供回退
  后端。
- **只做分层渲染。** 支持任意数量的 `ui::layer`，按稳定顺序绘制、可独立显示/隐藏。
  库不知道某层是 HUD、调试、菜单还是其他应用概念。
- **一个 ImGui 帧。** 所有层在同一个 ImGui 帧中绘制，最后提交一次 overlay pass。
- **唯一布局组件。** `ui::layout` 是可嵌套的 flex 容器；panel 只能通过 layout 加入层。
- **核心 flex 能力。** 支持 `flex-direction`、`flex-grow`、`flex-shrink`、`flex-basis`、
  子项 min/max、gap、padding 和 XY 双轴拉伸。
- **封装 ImGui 生命周期。** 应用不创建 ImGui 上下文、不调用 `NewFrame`/`Render`、不创建
  ImGui PSO 或字体纹理。
- **封装输入与字体。** 输入桥、CJK 字体、DPI、主题由 `ui` 统一处理。
- **可测试。** flex 布局、层顺序和帧生命周期可以在无 GPU 的测试中验证；真实 GPU 测试可选。
- **明确的所有权和异常语义。** 设备、窗口、system、layer、layout、panel 的生命周期和异常
  清理规则写进 API 文档。

### 3.2 非目标（v1）

- 不支持 Dear ImGui 之外的 UI 后端。
- 不支持 DiligentEngine 之外的渲染后端，也不提供 `renderer` 抽象接口。
- 不提供 HUD、diagnostics、menu、tooltip、pause 等应用语义的类型或预设。
- 不提供层级的 modal/capture 输入策略或自动输入阻挡。
- 不提供 anchor、stack、dock、grid、绝对定位或内容自适应尺寸。
- 不实现完整 flexbox：不支持 `flex-wrap`、`justify-content`、`align-items`、`align-self`、
  `order`、百分比、`flex-basis: auto`、baseline、margin auto。
- 不支持 docking、多 viewport、浮动/可拖动 panel、tab 或布局编辑器。
- 不支持一个进程多个 ImGui 上下文或多个 `ui::system`。
- 不做多线程 UI、异步字体加载或后台布局。
- 不在库内提供 diagnostics 面板、帧统计 UI 或调试窗口。

---

## 4. 总体架构

### 4.1 分层与依赖

```text
┌──────────────────────────────────────────────────────────────┐
│ 应用 / 游戏                                                   │
│  ├─ 世界渲染（render-core）                                   │
│  └─ UI 内容：ui::panel 实现，内部使用 ImGui 控件               │
└───────────────────────────────┬──────────────────────────────┘
                                │ add_layer / layout.add
┌───────────────────────────────▼──────────────────────────────┐
│ ui::system                                                    │
│  ├─ layer stack：ui::layer（按 order 排序，只认顺序/可见性）│
│  ├─ 每层一个根 ui::layout（flex 树）                           │
│  ├─ input bridge：platform::raw_input_state → ImGui IO        │
│  ├─ font / theme / DPI                                        │
│  └─ detail::system_impl                                       │
│         ├─ ImGui 上下文                                       │
│         └─ Diligent::ImGuiDiligentRenderer                    │
└───────────────────────────────┬──────────────────────────────┘
                                │ ui::frame_stage
┌───────────────────────────────▼──────────────────────────────┐
│ runtime::frame_pipeline                                       │
│  poll → clock → prelude → update → render stages → present    │
└──────────────────────────────────────────────────────────────┘
```

依赖方向：

```text
runtime ─────────┐
render-core ─────┼──→ ui ──→ application
platform ────────┘
Dear ImGui ──────┘
DiligentEngine ──┘
```

- `render-core` 拥有 Diligent device、swap chain 和 `frame_context`；`ui` 不创建设备。
- `ui` 直接依赖 `platform`，因为输入、窗口逻辑尺寸、framebuffer scale 和光标都需要窗口；
  `render-core` 不暴露平台窗口。
- `ui` 直接依赖 Dear ImGui 和 DiligentEngine。没有 `ui::renderer` 接口，也没有
  “以后换后端”的预留层。
- 一个进程只有一份 ImGui 代码和上下文。应用不得再直接链接 `Diligent-Imgui` 或自行创建
  ImGui 上下文，否则会出现重复符号和输入/字体状态分裂。

### 4.2 关键设计决策

| # | 决策 | 理由 |
| --- | --- | --- |
| 1 | 固定 Dear ImGui + DiligentEngine，不做后端抽象 | 需求明确只支持这一套；抽象层只会增加维护成本。 |
| 2 | `ui::layer` 是 UI 层，不是 runtime stage | 避免每层各开一帧或“首层 begin、末层 end”的脆弱协议；由单一 `ui::frame_stage` 统一绘制。 |
| 3 | `ui::system` 拥有上下文、渲染器、输入、字体和层栈 | 应用只关心内容；生命周期和 GPU 资源集中管理。 |
| 4 | 层只提供 order/visible，不提供应用语义 | HUD、diagnostics、menu 是应用概念，库不应内置枚举或预设。 |
| 5 | 层不提供 modal/capture 策略 | 输入策略是应用状态；库只报告全局 ImGui 捕获。应用可隐藏层或给 panel 加 `no_input`。 |
| 6 | 唯一布局是 `ui::layout` flex 容器 | 一个布局模型足够覆盖常见界面，避免 anchor/stack/fill 多套语义并存。 |
| 7 | 布局总是计算显式矩形，不做内容自适应 | flex 的 grow/shrink/basis 需要已知尺寸；panel 内容溢出由 panel 自己滚动/裁剪。 |
| 8 | 一个引擎帧一个 ImGui 帧 | ImGui 的输入、窗口排序、字体纹理和 draw data 都是帧级状态。 |
| 9 | UI 是独立的 overlay pass | ImGui 的顶点布局和动态缓冲与网格合批不同；直接使用 Diligent ImGui 渲染器。 |
| 10 | 一个进程一个 ImGui 上下文，单线程 | 与 ImGui 默认模型一致；不引入多上下文和隐藏线程。 |

### 4.3 公共头与内部实现

目标文件布局：

```text
ui/
  docs/
    roadmap.md
  src/
    cyberverse/
      ui.hpp
      ui/
        system.hpp
        layer.hpp
        panel.hpp
        panel_context.hpp
        layout.hpp
        frame_stage.hpp
        font.hpp
        theme.hpp
        imgui.hpp
        detail/
          system_impl.hpp
          input_bridge.hpp
          key_map.hpp
          layout_engine.hpp
          z_order.hpp
        legacy/            # 迁移期临时保留，M3 删除
          host.hpp
          panel.hpp
          frame_stage.hpp
  test/
  benchmark/
```

公共核心头不包含 `imgui.h` 或 Diligent 头。`cyberverse/ui/imgui.hpp` 是唯一的显式 ImGui
入口，供 panel 实现和需要定制 style 的应用使用。`detail/` 下的类型属于
`cyberverse::ui::detail`，不进入 umbrella header。

---

## 5. 核心对象设计

### 5.1 `ui::system`

`ui::system` 是应用侧唯一的 UI 门面，拥有：

- 唯一的 ImGui 上下文；
- 唯一的 `Diligent::ImGuiDiligentRenderer`；
- 输入桥和全局输入捕获状态；
- 字体图集、主题和 DPI 缩放；
- 有序的 `ui::layer` 层栈；
- 当前帧的 `render_core::frame_context` 快照。

目标 API：

```cpp
namespace cyberverse::ui {

struct system_config {
  bool headless = false;                         // true 时跳过 GPU 绘制，仅生成 draw data
  render_core::render_device* device = nullptr; // headless 时可为 null
  platform::window* window = nullptr;           // headless 时可为 null
  std::vector<font_config> fonts;               // 第一个是默认字体，后续可合并回退
  theme_kind theme = theme_kind::dark;
  bool keyboard_nav = true;
  bool persist_ini = false;
  std::filesystem::path ini_path;
  bool enable_dpi_scaling = true;
};

class system {
public:
  static std::unique_ptr<system> create(const system_config& config);
  ~system();

  system(const system&) = delete;
  system(system&&) = delete;
  system& operator=(const system&) = delete;
  system& operator=(system&&) = delete;

  // 层管理
  frame_stage& add_layer(std::unique_ptr<frame_stage> layer);
  bool remove_layer(frame_stage& layer) noexcept;
  void clear_layers() noexcept;
  [[nodiscard]] frame_stage* find_layer(std::string_view name) noexcept;
  [[nodiscard]] bool has_visible_content() const noexcept;

  // 帧生命周期
  bool begin_frame(const render_core::frame_context& frame, double delta_time);
  bool begin_frame(float display_width, float display_height, double delta_time); // headless
  void draw();
  void end_frame();
  [[nodiscard]] bool frame_active() const noexcept;

  // 全局输入捕获（反映最近一次完成的 UI 帧）
  [[nodiscard]] bool wants_mouse() const noexcept;
  [[nodiscard]] bool wants_keyboard() const noexcept;

  // 字体 / 主题 / 缩放；这些调用必须发生在帧之间
  void reload_fonts();
  [[nodiscard]] bool font_loaded(std::string_view id) const noexcept;
  void set_theme(theme_kind theme);
  void set_scale(float scale);
  [[nodiscard]] float scale() const noexcept;

  // 显式逃生口；需要 ImGui 类型时包含 cyberverse/ui/imgui.hpp
  [[nodiscard]] ImGuiContext* imgui_context() noexcept;
  [[nodiscard]] ImGuiIO& io() noexcept;
  [[nodiscard]] ImGuiStyle& style() noexcept;

private:
  struct impl;
  std::unique_ptr<impl> impl_;
};

} // namespace cyberverse::ui
```

`font_config` / `theme_kind` 的定义见 5.8；`frame_stage` 见 5.2；`ImGuiContext` / `ImGuiIO` /
`ImGuiStyle` 需要包含 `cyberverse/ui/imgui.hpp`。`system` 使用 pimpl 隐藏 Diligent/ImGui
头，但这不是后端抽象：`impl` 内部固定持有 `Diligent::ImGuiDiligentRenderer`。

生命周期规则：

- `system::create()` 先创建 ImGui 上下文，再创建字体图集和 `ImGuiDiligentRenderer`；任何一步
  失败都抛出异常并清理已创建的对象。
- `system` 必须在 `render_device` 和 `platform::window` 之前销毁；`ui::frame_stage` 必须在
  `system` 之前从 runtime 中移除。
- `begin_frame()` 失败（frame 无效、窗口尺寸为零、已有一帧活跃）返回 `false`；此时不得调用
  `draw()` 或 `end_frame()`。
- `begin_frame(frame, dt)` 在有窗口时使用 `window.width()/framebuffer_scale` 作为逻辑显示
  尺寸；窗口为空时使用 `frame.current_viewport()` 的宽高，scale 固定为 1。非 headless 模式
  还要求 `frame.device_context()` 和 `frame.color_target()` 非空。v1 假设 viewport 覆盖整个
  窗口内容区；子视口/离屏目标的输入坐标偏移属于后续能力。
- `begin_frame(width, height, dt)` 只用于 headless/测试，不需要 GPU，只生成 draw data；
  生产渲染使用 frame 重载。
- `begin_frame()` 成功后才允许 `draw()`；`draw()` 每帧只调用一次，负责按顺序绘制所有层。
- `end_frame()` 在所有层绘制完成后调用，负责 `ImGui::Render()` 和一次 overlay pass。未开启
  帧时调用是 no-op。
- 析构时如果仍有活跃帧，丢弃该帧并调用 `ImGui::EndFrame()`，不抛异常。
- 层/panel/layout 的结构修改只能在帧之间进行；`begin_frame()` 之后到 `end_frame()` 之前
  修改结构是逻辑错误（断言或 `std::logic_error`）。
- 所有方法必须在渲染线程调用；`ui::system` 不是线程安全的。

Headless 模式：

- `headless == true` 时不构造 `Diligent::ImGuiDiligentRenderer`，只创建 ImGui 上下文、
  处理输入、生成 draw data；`end_frame()` 跳过 `RenderDrawData()`。它用于布局、层顺序和
  帧生命周期的无 GPU 测试。
- ImGui 1.92 的字体纹理需要 texture ID；headless 测试使用一个仅测试可见的 texture stub，
  不创建 GPU 资源。这个 stub 不是可替换渲染后端，也不进入公共 API。
- `window == nullptr` 时从 `frame.current_viewport()` 或 `begin_frame(width, height, dt)` 的
  参数推导显示尺寸，scale 固定为 1；没有平台输入，测试通过 `platform` 的 headless 窗口注入
  事件（见 5.5）。
- 生产路径必须提供 `device` 和 `window`；`create()` 在任一缺失且 `headless == false` 时抛出
  `std::invalid_argument`。

### 5.2 `ui::layer`（UI 层）

`ui::layer` 表示 **一层 UI**。它只知道自己的名称、顺序、可见性和根布局，不知道层里
画的是什么应用内容。

目标 API：

```cpp
namespace cyberverse::ui {

struct layer_desc {
  std::string name;
  int order = 0;
  bool visible = true;
};

class layer {
public:
  explicit layer(layer_desc desc);
  ~layer();

  layer(const layer&) = delete;
  layer(layer&&) = delete;
  layer& operator=(const layer&) = delete;
  layer& operator=(layer&&) = delete;

  [[nodiscard]] std::string_view name() const noexcept;
  [[nodiscard]] int order() const noexcept;
  void set_order(int order);
  [[nodiscard]] bool visible() const noexcept;
  void set_visible(bool visible);

  panel& add_panel(std::unique_ptr<panel> content);
  bool remove_panel(panel& content) noexcept;
  void clear_panels();
  [[nodiscard]] bool has_visible_panels() const noexcept;

private:
  // ...
};

} // namespace cyberverse::ui
```

层语义：

- **顺序**：`order` 小的先绘制（在下面）。同 order 按加入顺序稳定排序。层顺序每帧缓存，
  只有增删或 `set_order()` 后重建，不在热路径排序分配。
- **可见性**：`visible == false` 的层完全跳过；如果所有层都没有可见 panel，
  `ui::frame_stage` 不开启 ImGui 帧，也不调用 `device.begin_frame()`。
- **唯一内容入口**：层的内容只能通过 `root_layout()` 添加。没有 `add_panel()`，因此布局
  不会被绕过。
- **没有应用语义**：层没有输入策略、背景、margin 或应用层预设；应用按自己的需要设置
  `order` 和 `visible`。
- **没有背景/遮罩**：层本身透明；需要全屏背景或遮罩时，在层内添加一个铺满的 panel，由
  panel 自己绘制。
- **没有更新逻辑**：层没有 `on_update`。应用在 `on_frame_begin` / update 阶段修改可见性、
  panel 状态和业务数据；层只负责绘制。
- **名称校验**：`add_layer` 拒绝空名和重复层名，抛出 `std::invalid_argument`。

### 5.3 `ui::panel`

panel 只负责内容，窗口和矩形由 layout/层创建和管理。

```cpp
namespace cyberverse::ui {

struct rect {
  float x = 0.0f;
  float y = 0.0f;
  float width = 0.0f;
  float height = 0.0f;
};

// 正向特性：默认窗口没有标题栏、无滚动条、有背景、可交互、不抢焦点。
enum class panel_feature : std::uint32_t {
  none = 0,
  title_bar = 1u << 0,
  scrollbar = 1u << 1,
  transparent = 1u << 2,             // 不画窗口背景
  no_input = 1u << 3,                // 窗口只显示，不接收输入
  focus_on_appearing = 1u << 4,
  bring_to_front_on_focus = 1u << 5,
};

constexpr panel_feature operator|(panel_feature a, panel_feature b) noexcept;
constexpr panel_feature operator&(panel_feature a, panel_feature b) noexcept;

struct panel_context {
  const system& ui;
  const frame_stage& stage;
  rect bounds;                   // 布局分配给 panel 的完整窗口矩形（逻辑点）
  double delta_time = 0.0;
  std::uint64_t frame_index = 0;
  float scale = 1.0f;
};

class panel {
public:
  virtual ~panel() = default;

  // 层内唯一、跨帧稳定；建议使用 "app.status" 这类命名
  [[nodiscard]] virtual std::string_view id() const noexcept = 0;

  // 可选的窗口标题；空表示无标题
  [[nodiscard]] virtual std::string_view title() const noexcept { return {}; }

  [[nodiscard]] virtual bool visible() const noexcept { return true; }

  [[nodiscard]] virtual panel_feature features() const noexcept { return panel_feature::none; }

  // 在层已经 Begin 的固定矩形窗口内绘制内容
  virtual void draw(panel_context& context) = 0;
};

} // namespace cyberverse::ui
```

窗口管理规则（由层/布局内部完成）：

- 窗口名使用 `"标题###<层名>/<panel-id>"`；即使标题动态变化，ImGui ID 也保持稳定。层名、
  panel id 和标题不得包含 `###`。
- panel id 必须在层内唯一；应用负责保证，层在 debug 构建中计算 placement 时断言。
- 窗口位置和尺寸**总是**由布局计算：`ImGui::SetNextWindowPos` + `ImGui::SetNextWindowSize`。
  不使用 `ImGui::SetNextWindowSizeConstraints` 或 `AlwaysAutoResize`。
- 默认 flags：`NoTitleBar | NoResize | NoMove | NoCollapse | NoScrollbar | NoSavedSettings |
  NoBringToFrontOnFocus`。`features()` 映射为：
  - `title_bar` → 去掉 `NoTitleBar`；
  - `scrollbar` → 去掉 `NoScrollbar`；
  - `transparent` → 加 `NoBackground`；
  - `no_input` → 加 `NoInputs`；
  - `focus_on_appearing` → 去掉 `NoFocusOnAppearing`；
  - `bring_to_front_on_focus` → 去掉 `NoBringToFrontOnFocus`（层 z-order 保护仍优先）。
- `ImGui::Begin()` 返回 false（裁剪等）时仍然调用 `ImGui::End()`，并跳过内容。
- 层不自动绘制关闭按钮；panel 的可见性由 `visible()` 或应用状态决定。需要关闭按钮时，
  panel 在内容里自行绘制并修改应用状态。
- 层负责 `ImGui::PushID(panel.id())` / `PopID()`，避免不同 panel 的内部控件 ID 冲突。
- panel 抛异常时，层必须结束当前窗口、标记帧失败并向上传播；`ui::system` 在异常路径调用
  `ImGui::EndFrame()` 丢弃该帧（不调用 renderer），下一帧重新开始。
- panel 不拥有窗口，也不应保存 `panel_context`；它只在 `draw()` 调用期间有效。
- 内容溢出由 panel 自己处理：加 `scrollbar` 特性、使用 `BeginChild` 或裁剪；布局不会因为
  内容变高而改变矩形。
- v1 不提供“panel 自己管理顶层窗口”的模式。浮动调试窗口属于后续 `free_panel` 能力。

### 5.4 `ui::layout`（唯一的 flex 布局组件）

`ui::layout` 是一个可嵌套的 flex 容器，也是层内容的唯一入口。它负责：

- 根据父容器矩形计算每个子项的显式矩形；
- 支持 `flex-direction`；
- 支持 `flex-grow` / `flex-shrink` / `flex-basis`；
- 支持子项 `min_width` / `min_height` / `max_width` / `max_height`；
- 主轴按 flex 分配，交叉轴默认拉伸，即 XY 双轴都填满可用空间（受 min/max 限制）；
- 支持 gap 和 padding。

#### 5.4.1 基础类型

坐标统一为 **逻辑点**，原点在窗口内容区左上角，+Y 向下。

```cpp
namespace cyberverse::ui {

struct insets {
  float left = 0.0f;
  float top = 0.0f;
  float right = 0.0f;
  float bottom = 0.0f;
};

enum class flex_direction {
  row,
  row_reverse,
  column,
  column_reverse,
};

} // namespace cyberverse::ui
```

#### 5.4.2 flex 子项

```cpp
namespace cyberverse::ui {

struct flex {
  // 主轴剩余空间的分配比例；0 表示不参与拉伸
  float grow = 1.0f;

  // 主轴溢出时的收缩比例；0 表示不收缩
  float shrink = 1.0f;

  // 主轴初始尺寸（逻辑点）：row 时是宽度，column 时是高度；
  // 不是百分比，也不是内容自适应
  float basis = 0.0f;

  // 两个轴的最小/最大尺寸；默认无下限/上限
  float min_width = 0.0f;
  float min_height = 0.0f;
  float max_width = std::numeric_limits<float>::infinity();
  float max_height = std::numeric_limits<float>::infinity();
};

} // namespace cyberverse::ui
```

字段与 HTML flexbox 的对应：

| HTML 概念 | `ui::flex` 字段 | 本版语义 |
| --- | --- | --- |
| `flex-grow` | `grow` | 主轴剩余空间按比例分配。 |
| `flex-shrink` | `shrink` | 主轴溢出时按 `shrink * basis` 加权收缩。 |
| `flex-basis` | `basis` | 主轴初始尺寸，单位逻辑点。 |
| `min-width` / `min-height` | `min_width` / `min_height` | 子项两轴下限，参与 grow/shrink 的冻结。 |
| `max-width` / `max-height` | `max_width` / `max_height` | 子项两轴上上限，参与 grow/shrink 的冻结。 |
| `align-items: stretch` | 固定行为 | 交叉轴默认拉伸到容器交叉轴尺寸，受 min/max 限制。 |

默认值 `grow = 1, shrink = 1, basis = 0` 表示所有子项均分主轴空间；这比 HTML 的
`flex: 0 1 auto` 更适合没有内容测量阶段的立即模式布局。需要固定尺寸时显式设置
`grow = 0, shrink = 0, basis = <main_size>`。

#### 5.4.3 layout 树

```cpp
namespace cyberverse::ui {

class layout {
public:
  explicit layout(flex_direction direction = flex_direction::column);
  ~layout();

  layout(const layout&) = delete;
  layout(layout&&) = delete;
  layout& operator=(const layout&) = delete;
  layout& operator=(layout&&) = delete;

  void set_direction(flex_direction direction) noexcept;
  void set_gap(float gap) noexcept;
  void set_padding(insets padding) noexcept;

  [[nodiscard]] flex_direction direction() const noexcept;
  [[nodiscard]] float gap() const noexcept;
  [[nodiscard]] insets padding() const noexcept;

  // 添加 panel；返回的引用在 remove/clear 前有效
  panel& add(std::unique_ptr<panel> child, flex item = {});

  // 添加嵌套 layout；返回的引用在 remove/clear 前有效
  layout& add_layout(flex item = {});

  bool remove(panel& child) noexcept;
  bool remove(layout& child) noexcept;
  void clear() noexcept;

  bool set_flex(panel& child, flex item) noexcept;
  bool set_flex(layout& child, flex item) noexcept;

  [[nodiscard]] bool has_visible_panels() const noexcept;

private:
  // ...
};

} // namespace cyberverse::ui
```

- 每个 `frame_stage` 有一个根 `layout`；根布局默认 `column`、`gap = 0`、`padding = 0`。
- `layout` 可以嵌套；嵌套布局作为父布局的 flex 子项，父布局保存其 `flex`。
- `layout` 拥有 panel 和子 layout；删除/清空后对应引用失效。
- panel 的 `visible() == false` 时从布局中排除，兄弟项重新分配空间。
- layout 本身没有 `visible`；需要隐藏整棵子树时，应用可以清除它或把它的 `grow` 设为 0
  （但空布局仍可能占用 gap/basis）。v1 不提供 layout 可见性。
- `has_visible_panels()` 递归检查子树；用于跳过空层。

#### 5.4.4 布局算法

对每个 layout 节点：

1. 从父布局获得容器矩形 `container`；减去 `padding` 得到内容矩形 `content`。
2. 收集可见子项（panel 的 `visible()` 为 true；嵌套 layout 总是参与）。
3. 主轴是 `row` / `row_reverse` 的水平方向，或 `column` / `column_reverse` 的垂直方向；
   交叉轴是另一轴。
4. 子项数为 0 时直接返回；否则主轴可用尺寸 = 内容主轴尺寸 - `gap * (子项数 - 1)`。
5. 每个子项的初始主轴尺寸 `base = clamp(basis, min_main, max_main)`。
6. 计算剩余空间 `free = 主轴可用尺寸 - sum(base)`：
   - `free > 0`：按 `grow` 比例分配；每次分配后把超过 `max_main` 的子项冻结在 max，剩余
     空间继续分配给未冻结子项；`grow` 总和为 0 时保留剩余空间（主轴起点对齐）。
   - `free < 0`：按 `shrink * base` 加权收缩；每次收缩后把低于 `min_main` 的子项冻结在 min，
     剩余亏空继续由未冻结子项承担；`shrink * base` 总和为 0 时允许溢出。
   - `free == 0`：保持 base。
7. 交叉轴尺寸 = 内容交叉轴尺寸，再 clamp 到 `[min_cross, max_cross]`。这是“XY 双轴拉伸”：
   主轴由 flex 填满，交叉轴默认拉伸填满；如果 `max_cross` 小于容器，则子项不会超过 max，
   从交叉轴起点对齐。
8. 按方向放置子项：
   - `row`：从左到右；
   - `row_reverse`：从右到左；
   - `column`：从上到下；
   - `column_reverse`：从下到上；
   - 相邻子项之间加入 `gap`。
9. 对嵌套 layout 递归执行 1–8；对 panel 生成一个 placement（panel 指针 + 矩形）。
10. 反向方向只改变位置，不改变子项的绘制顺序；层内 z-order 仍按添加顺序。

数值清洗：

- `grow` / `shrink` / `gap` / `padding` 的负值按 0 处理；
- `basis` 非有限值按 0；
- `min > max` 时把 `max` 提升到 `min`，避免负尺寸；
- 容器尺寸为 0 或负数时，子项尺寸为 0；
- 浮点误差不应产生负宽高，最终矩形统一 clamp 到非负。

已明确不支持：

- `flex-wrap`、多行/多列；
- `justify-content` / `align-items` / `align-self` / `order`；
- 百分比 `basis`、`flex-basis: auto`、内容测量；
- `margin` / `margin: auto`；
- baseline、aspect-ratio、grid、dock；
- panel 自动尺寸。

居中、右对齐等需求可以通过嵌套 layout + 空 spacer 子项实现，也可以由 panel 内容自己用
ImGui 光标/子窗口完成；布局本身只保证 flex 分配和双轴拉伸。

#### 5.4.5 布局与 ImGui 窗口

- 布局引擎输出显式 `rect`，层用 `ImGui::SetNextWindowPos` / `SetNextWindowSize` 创建窗口。
- 所有 panel 都是固定矩形窗口；内容超出时由 panel 自己滚动或裁剪。
- 布局不读取 ImGui 的内容尺寸，也不根据 `ImGui::GetWindowSize()` 反推布局；这保证了
  布局在无 GPU、无 ImGui 绘制的情况下也可以单独测试。
- 布局引擎可以只输出 placement 列表，不直接调用 ImGui；层负责绘制。这样 flex 算法是纯函数，
  易于表驱动测试。

### 5.5 输入桥

输入桥把 `platform::raw_input_state` 转换为 ImGui 事件，并维护全局捕获状态。

```text
platform::raw_input_state
  ├─ 鼠标位置 / 按键 / 滚轮
  ├─ 键盘 down / pressed / released
  ├─ 修饰键
  └─ UTF-8 文本
        │
        ▼
detail::input_bridge
  ├─ key_id → ImGuiKey 映射
  ├─ AddMousePosEvent / AddMouseButtonEvent / AddMouseWheelEvent
  ├─ AddKeyEvent / AddInputCharactersUTF8
  └─ 焦点丢失时释放所有键
        │
        ▼
ImGuiIO
```

规则：

- **坐标**：`platform` 的鼠标位置是窗口内容坐标（逻辑点），直接作为 ImGui 坐标使用；不要
  再乘 `framebuffer_scale`。
- **键盘**：`platform::key_id` 的数值对齐 GLFW key code；输入桥用显式表映射到 `ImGuiKey`。
  覆盖字母、数字、标点、功能键、方向键、编辑键、小键盘、修饰键和菜单键；未映射的键忽略。
- **修饰键**：同时发送 `ImGuiMod_Ctrl/Shift/Alt/Super` 和左右修饰键事件，行为与 ImGui 官方
  GLFW 后端一致。
- **文本**：将 UTF-8 解码为 code point 后调用 `ImGui::AddInputCharacter`；避免为每帧文本创建
  临时 `std::string`。IME 组合输入不在 v1 范围。
- **滚轮**：平台滚轮按“步”给出时直接发送；触控板像素滚动需要可配置的缩放系数。v1 提供
  `scroll_scale` 配置，默认 1.0。
- **焦点丢失**：平台窗口失焦时会 `release_all()`；输入桥每帧比较平台按键状态与上一帧，补发
  key-up，避免 ImGui 中按键卡住。v1 不要求平台新增 focus 事件；如果后续需要精确的
  `AddFocusEvent`，再扩展 platform。
- **全局捕获**：`wants_mouse()` / `wants_keyboard()` 在 `end_frame()` 后更新，反映刚刚绘制的
  这一帧。应用在下一帧读取，存在一帧延迟。
- **没有层策略**：`ui` 不决定哪一层拥有输入。应用通过隐藏层、panel 的 `no_input` 特性和
  自己的游戏状态来管理输入独占/层级行为。
- **光标**：v1 不自动改变平台光标模式。需要显示/隐藏光标时由应用设置
  `platform::window::set_cursor_mode`。
- **测试**：使用 `platform` 的 headless 窗口注入事件，断言 ImGui IO 的按键、鼠标、文本和
  `WantCaptureMouse/Keyboard`；key map 表本身做穷举测试。

### 5.6 `ui::frame_stage`

`ui::frame_stage` 是 `ui` 与 `runtime` 的唯一适配器。它继承 `runtime::frame_stage`，
variable-step，`on_update` 为空；所有 UI 绘制集中在一次 `on_render` 中完成。

```cpp
namespace cyberverse::ui {

using frame_provider = std::function<render_core::frame_context(
    double delta_time, double alpha, const runtime::frame_budget& budget)>;

class frame_stage final : public runtime::frame_stage {
public:
  frame_stage(system& ui, render_core::render_device& device);
  frame_stage(system& ui, frame_provider provider);
  ~frame_stage() override;

  void on_update(double dt, std::uint64_t tick, const runtime::frame_budget& budget) override;
  void on_render(double dt, double alpha, const runtime::frame_budget& budget) override;

private:
  system* ui_ = nullptr;
  frame_provider provider_;
};

} // namespace cyberverse::ui
```

语义：

- 默认构造函数使用 `device.begin_frame(dt, alpha, budget)` 获取 `frame_context`；第二个构造
  函数允许应用共享已有的 frame context，或在测试中注入一个只有 viewport 的 detached frame。
- `on_render`：
  1. 如果 `ui_->has_visible_content()` 为 false，直接返回，不调用 `device.begin_frame`；
  2. `auto frame = provider_(dt, alpha, budget);`
  3. `ui_->begin_frame(frame, dt)`；失败则返回；
  4. `ui_->draw()`；
  5. `ui_->end_frame()`。
- 异常从 panel 或 renderer 传播出去；`frame_pipeline::step_frame` 不会记录该帧统计。`system`
  负责在异常路径结束 ImGui 帧。
- 该 stage 必须注册在所有 3D 渲染 stage 之后，`render_order` 取较高值；`receive_update =
  false`，`receive_render = true`。一个应用只注册一个 `ui::frame_stage`。
- 如果应用需要在 UI 之后继续渲染，必须自行恢复 viewport/scissor/render targets；v1 推荐 UI
  作为最后一个渲染阶段。

### 5.7 渲染后端

`ui` 固定使用 `Diligent::ImGuiDiligentRenderer`，没有 `ui::renderer` 接口，也没有可替换
后端。

```text
ui::system::impl
  ├── ImGui 上下文
  └── Diligent::ImGuiDiligentRenderer
```

- `system::impl` 用 `render_core::render_device::device()` 和 swap chain 的 color/depth
  format 构造 `ImGuiDiligentRenderer`。
- 每帧顺序：`renderer.NewFrame(framebuffer_width, framebuffer_height,
  SURFACE_TRANSFORM_IDENTITY)` → 输入桥 → `ImGui::NewFrame()` → 布局/层绘制 →
  `ImGui::Render()` → `renderer.RenderDrawData(device_context, ImGui::GetDrawData())`。
- UI 是独立的 overlay pass，不经过 `render-core` 的 `mesh_phrase` / `pipeline_manager`。这是
  有意的设计：ImGui 的顶点布局、动态顶点/索引缓冲和 PSO 与网格合批不同。`ui` 仍然使用
  render-core 的设备、上下文、颜色目标和 viewport。
- `ui` 不调用 `present()`；present 由 runtime/应用在 UI pass 之后执行。
- 渲染完成后重置 scissor 到完整目标，避免 ImGui 最后的 scissor 影响后续 pass。UI 仍应作为
  最后一个渲染阶段；如果未来需要在 UI 后继续绘制，必须显式恢复 viewport/scissor/render
  targets。
- 交换链 resize 由 `render_core::render_device::resize()` 处理；UI 每帧从 `frame_context` /
  `window` 读取新尺寸，`ImGuiDiligentRenderer::NewFrame` 接收新的 framebuffer 尺寸，不需要
  重建 PSO。
- 设备丢失/重置不在 v1 保证；预留 `system::on_device_lost()` / `on_device_reset()` 接口，
  内部调用 renderer 的 `InvalidateDeviceObjects()` / `CreateDeviceObjects()`，在 M6 实现。
- 析构顺序必须是：renderer `InvalidateDeviceObjects()` → 销毁 renderer → 销毁 ImGui
  上下文；否则 ImGui 纹理/SRB 可能在 device 之后释放。
- 颜色转换使用 `IMGUI_COLOR_CONVERSION_MODE_AUTO`；render-core 默认 sRGB 交换链，自动走
  sRGB→linear 路径。
- 一个进程只允许一份 `Diligent-Imgui` / ImGui。`ui` 的 CMake target 会把 ImGui include 目录
  作为 PUBLIC 暴露，但应用不得再链接第二份 ImGui。
- CMake 必须直接链接 `Diligent-Imgui`；如果目标不存在，配置阶段失败并提示
  `CYBERVERSE_DILIGENT_DIR` / `DILIGENT_BUILD_TOOLS`，不回退到其他后端。

### 5.8 字体、DPI 与主题

字体配置：

```cpp
namespace cyberverse::ui {

enum class glyph_range {
  default_latin,
  chinese_simplified_common,
  custom,
};

enum class theme_kind { dark, light };

struct font_config {
  std::string id;                  // 例如 "default" / "cjk"
  std::filesystem::path path;      // 空表示 ImGui 内置默认字体
  float size = 16.0f;              // 逻辑点
  glyph_range range = glyph_range::default_latin;
  std::vector<char32_t> glyphs;    // range == custom 时使用
  bool merge = false;              // 合并到前一个字体（回退字体）
  int oversample_h = 1;
  int oversample_v = 1;
  bool pixel_snap = false;
};

} // namespace cyberverse::ui
```

- 第一个 `font_config` 是默认字体；`merge == true` 的后续字体合并到前一个字体，用于 CJK
  回退。`path` 为空且不是默认字体的条目视为未配置，跳过并令 `font_loaded(id) == false`。
- `ui::system::font_loaded(id)` 返回该字体是否成功加载；应用用它在中文字体缺失时回退英文
  文案。
- 字体图集在 `create()` 或 `reload_fonts()` 时构建。ImGui 1.92 的动态纹理路径由
  `ImGuiDiligentRenderer` 处理；如果使用的 Diligent 版本不支持，降级为显式
  `Fonts->Build()` + 纹理重建。`reload_fonts()` 后旧 `ImFont*` 可能失效，panel 不应长期
  缓存字体指针，或应在 reload 后重新获取。
- 提供 `ui::find_system_cjk_font()` 辅助函数，按平台尝试常见 CJK 字体路径；找不到时返回空，
  由应用决定回退文案。库本身不硬编码“必须存在某个系统字体”。
- 字体文件许可由应用负责。

DPI 策略（推荐，M5 用真实截图验证）：

- 布局和 ImGui 坐标使用逻辑点；`io.DisplaySize` 为逻辑窗口尺寸。`set_scale()` 更新逻辑点到
  物理像素的比例，并可能触发字体图集重建；只能在帧之间调用。
- 字体按 `font.size * framebuffer_scale` 栅格化，`io.FontGlobalScale = 1 / framebuffer_scale`，
  使逻辑字号不变、字形按物理像素清晰渲染。
- `style` 的尺寸保持逻辑点；如果验证发现控件视觉尺寸不一致，再改为 `ScaleAllSizes`。
- 鼠标坐标使用平台逻辑坐标，不乘 scale。
- `enable_dpi_scaling == true` 时，system 在帧间检测 `window.framebuffer_scale()` 变化并更新
  scale/字体；应用也可以显式 `set_scale()` 覆盖。
- 在 macOS Retina（2x）和 Windows 150% 缩放下做截图对比；如果 ImGui 当前版本的
  `DisplayFramebufferScale` 行为与预期不同，M5 允许调整为“framebuffer 像素坐标系”，但必须
  保持应用侧布局单位一致。

主题：

- `theme_kind::dark` / `light` 提供中性预设；应用可以通过 `system.style()` 定制。
- `set_theme()` 会覆盖当前 style；应用自定义 style 后不要再调用它。
- v1 不做运行时 style 编辑器。

---

## 6. 帧流程

### 6.1 典型顺序

```text
runtime::frame_pipeline::step_frame
  ├─ poll（platform 窗口事件）
  ├─ clock tree update
  ├─ preludes（render-core 的 core_frame_prelude 等）
  ├─ pipeline / stage on_frame_begin
  ├─ stage updates（游戏逻辑；应用可在此更新 UI 状态）
  ├─ render frame begin
  ├─ 世界 render stages（render-core 绘制 3D）
  ├─ ui::frame_stage::on_render          ← 唯一 UI 适配器
  │    ├─ 检查 has_visible_content()
  │    ├─ device.begin_frame(dt, alpha, budget)
  │    ├─ system.begin_frame(frame, dt)
  │    │    ├─ renderer.NewFrame(...)
  │    │    ├─ input bridge → ImGuiIO
  │    │    └─ ImGui::NewFrame()
  │    ├─ system.draw()
  │    │    ├─ 按 order 遍历可见层
  │    │    ├─ 每层：root_layout.compute(viewport) → placement 列表
  │    │    ├─ 每个 panel：SetNextWindowPos/Size → Begin → draw → End
  │    │    └─ 层 z-order 保护
  │    └─ system.end_frame()
  │         ├─ ImGui::Render()
  │         ├─ renderer.RenderDrawData(device_context, draw_data)
  │         └─ 重置 scissor / 更新捕获状态
  ├─ 其他 render stages（如果存在，必须自行恢复状态）
  ├─ render frame end
  └─ present
```

### 6.2 层顺序与可见性

| 场景 | 库的行为 | 应用的责任 |
| --- | --- | --- |
| 多个层同时可见 | 按 order 从下到上绘制；每层独立 flex 布局。 | 决定层的内容和顺序。 |
| 某层隐藏 | 完全跳过该层，不计算布局、不绘制 panel。 | 在帧之间设置 `visible`。 |
| 某层所有 panel 隐藏 | 该层不产生 placement；只有所有层都没有可见 panel 时，`has_visible_content()` 才为 false，不开启空帧。 | 用 panel 的 `visible()` 控制。 |
| 需要“模态”效果 | 库不提供 modal；仍只按 order 绘制。 | 隐藏下层、给下层 panel 加 `no_input`、停止游戏输入。 |
| 需要焦点/输入捕获 | 报告全局 `wants_mouse()` / `wants_keyboard()`。 | 结合自己的 UI 状态决定是否处理游戏输入。 |

- 层顺序只决定绘制顺序；不决定输入归属。输入归属由 ImGui 的窗口顺序/悬停和应用的层
  可见性共同决定。
- 应用若需要严格的“上层独占输入”，可以在打开上层时隐藏下层，或把下层 panel 设为
  `no_input`；库不内置 modal 状态机。
- 同一层内 panel 的绘制顺序是布局树的深度优先添加顺序；后添加的 panel 在重叠时位于上层。
  `flex-direction: reverse` 只改变位置，不改变绘制顺序。

### 6.3 层 z-order

ImGui 的窗口顺序是全局的，而层顺序是 UI 自己的概念；两者需要对齐：

- 所有 UI panel 默认加 `ImGuiWindowFlags_NoBringToFrontOnFocus`，避免点击下层窗口时它跳到
  上层。
- 层按 order 从下到上绘制；每层绘制结束后，`detail::z_order` 把该层的窗口提升到更低层窗口
  之上。实现可以调用 `ImGui::BringWindowToDisplayFront`（`imgui_internal.h`），并集中在一个
  文件中做版本检查。
- 如果项目决定不使用 ImGui 内部 API，降级约定是：重叠层不保证严格 z-order；应用应通过
  可见性/`no_input` 避免同时显示重叠层。该降级方案写入 README，并在 M2 评审时最终确认。
- 备选方案“每层一个 ImGui 上下文”被拒绝：会复制字体、输入、后端和 draw data，成本远高于
  一个集中的 z-order 保护。

---

## 7. 关键不变量

实现和测试必须保持以下不变量：

1. **固定依赖。** `ui` 只支持 Dear ImGui + DiligentEngine；没有后端接口、后端选择或替代
   渲染路径。
2. **一个引擎帧最多一次 `ImGui::NewFrame()` 和一次 `ImGui::Render()`。** 由
   `ui::system::begin_frame()` / `end_frame()` 保证；`ui::frame_stage` 是唯一正常入口。
3. **层是通用的。** 库不定义 HUD/diagnostics/menu 类型、枚举、预设或诊断面板；层只有
   name/order/visible/root layout。
4. **唯一布局。** 所有 panel 都必须通过 `ui::layout` 加入层；没有 anchor/stack/fill/absolute
   的第二条布局路径。
5. **显式矩形。** 布局总是计算 panel 的 x/y/width/height；不使用内容自适应尺寸或
   `AlwaysAutoResize`。
6. **XY 双轴拉伸。** 主轴由 grow/shrink/basis 填满；交叉轴默认拉伸到容器尺寸，并受 min/max
   限制。
7. **层顺序稳定。** order 小的先画；同 order 按加入顺序；只有增删/改 order 才重建排序缓存。
8. **`begin_frame` / `draw` / `end_frame` 成对。** `begin_frame` 返回 false 时不得 draw/end；
   未开启帧时 `end_frame` 是 no-op。
9. **所有 UI 层共享同一个 ImGui 上下文和字体图集。** 不创建第二份 ImGui。
10. **单线程。** 所有 UI 调用在渲染线程；没有隐藏线程、异步字体加载或后台布局。
11. **一个进程一份 ImGui。** 应用不得再链接 `Diligent-Imgui` 或创建自己的 ImGui 上下文。
12. **生命周期顺序。** `frame_stage` 先移除，`system` 再销毁，最后才是 `render_device` /
    `platform::window`。
13. **异常安全。** panel 抛异常时当前窗口必须 `End`，帧必须 `EndFrame` 清理；异常继续传播，
    runtime 不会记录该帧统计。
14. **UI 是最后一个渲染阶段。** 如果 UI 之后还有 pass，必须显式恢复 viewport/scissor/render
    targets；v1 不提供自动恢复整条管线状态。

---

## 8. 路线图

### 8.1 里程碑总览

| 里程碑 | 目标 | 规模 | 依赖 | 主要风险 |
| --- | --- | --- | --- | --- |
| M0 | 规则、固定依赖、公共头骨架 | S | 无 | `Diligent-Imgui` 目标解析 |
| M1 | ImGui + Diligent 生命周期、字体/样式基线 | M | M0 | ImGui 版本/纹理 API |
| M2 | 通用层、panel、runtime 适配器、单帧模型 | M | M1 | 层 z-order、异常清理 |
| M3 | flex 布局 + sandbox 迁移 | L | M2 | flex 算法、固定矩形适配 |
| M4 | 完整输入桥与全局捕获 | M | M2、M3 | 键位覆盖、焦点释放 |
| M5 | 字体/CJK/DPI/主题 | M | M3、M4 | Retina/字体图集 |
| M6 | 健壮性、测试、文档和发布 | M | 全部 | 设备重置、跨平台 |

规模是粗略的人日量级：S ≈ 1–2 天，M ≈ 3–5 天，L ≈ 5–8 天；不作为排期承诺。

### 8.2 M0 — 规则、固定依赖与骨架

**目标：** 让仓库规则和新方向一致，建立可编译的新公共 API 骨架，同时保持 sandbox 可构建。

交付物：

- 本路线图。
- 更新 `ui/CLAUDE.md`、`ui/README.md` 和工作区 `AGENTS.md`：
  - `ui` 固定依赖 ImGui + DiligentEngine，不再是 backend-agnostic；
  - 只提供分层渲染，不提供 HUD/diagnostics/menu 预设；
  - 唯一的 `ui::layout` flex 布局；
  - 删除旧 anchor/stack/fill、modal/capture、diagnostics 描述。
- 旧 API 临时移入 `cyberverse::ui::legacy`：
  - `legacy::host`、`legacy::panel`、`legacy::frame_stage`；
  - sandbox 临时改用 legacy 头，保证主分支可构建；
  - 旧测试保留并标记 legacy，M3 删除。
- CMake：
  - 新增 `cmake/engine_dep_render_core.cmake`、`cmake/engine_dep_platform.cmake`（如需要）；
  - 直接解析并链接 `cyberverse::render_core::static_lib`、
    `cyberverse::platform::static_lib`、`cyberverse::runtime::static_lib` 和
    `Diligent-Imgui`；
  - `Diligent-Imgui` 不存在时配置失败，并提示 `CYBERVERSE_DILIGENT_DIR` /
    `DILIGENT_BUILD_TOOLS`；不提供后端回退选项；
  - 增加 `CYBERVERSE_UI_BUILD_TESTS` / `CYBERVERSE_UI_BUILD_BENCHMARKS` 选项。
- 公共头骨架：`system.hpp`、`layer.hpp`、`panel.hpp`、`panel_context.hpp`、
  `layout.hpp`、`frame_stage.hpp`、`font.hpp`、`theme.hpp`、`imgui.hpp`；更新 umbrella
  `cyberverse/ui.hpp`。不创建 `diagnostics.hpp`。
- 测试骨架：flex 布局、层排序、输入映射、system 生命周期的空测试 target。

验收：

- 在仓库外配置和构建成功：
  `cmake -S . -B /tmp/cyberverse-ui-test && cmake --build /tmp/cyberverse-ui-test`。
- `ctest` 能运行（允许新测试为空/占位）。
- sandbox-demo 仍能构建（使用 legacy 头）。
- `ui` target 链接到 `Diligent-Imgui`，没有重复 ImGui 符号。
- 仓库中没有“可切换 UI/渲染后端”的选项或文档。
- README/AGENTS 不再与目标架构矛盾。

风险与缓解：

- `Diligent-Imgui` 只在 render-core 的源码构建中存在；安装包场景可能没有该 target。
  M0 先支持工作区 sibling 构建，安装/导出留到 M6；配置错误必须指出
  `CYBERVERSE_DILIGENT_DIR` 或 `DILIGENT_BUILD_TOOLS`。
- 旧/新 `frame_stage` 同名冲突：通过 `legacy` 命名空间隔离，M3 删除。

### 8.3 M1 — ImGui + Diligent 生命周期

**目标：** 由 `ui` 自己创建和管理 ImGui 上下文、Diligent ImGui 渲染器、字体和样式，能够绘制
一个空帧。

交付物：

- `ui::system::create()` / 析构：
  - `ImGui::CreateContext()` / `DestroyContext()`；
  - 直接构造 `Diligent::ImGuiDiligentRenderer`，不引入 `ui::renderer` 接口；
  - 默认字体和 style 初始化；
  - `io.IniFilename` / `LogFilename` 默认禁用，`persist_ini` 可选。
- `ui::system::begin_frame` / `end_frame` 的基础版本：设置 `DisplaySize`、
  `DisplayFramebufferScale`、`DeltaTime`，调用 `NewFrame` / `Render`。
- Headless 测试路径：跳过 `RenderDrawData`，用测试专用 texture stub 满足 ImGui 字体纹理；
  该 stub 不是公共后端。
- 最小输入桥（鼠标位置、鼠标按键、滚轮、常用键盘键、文本）以支撑可交互 smoke test；
  M4 再补全映射。
- 异常清理：`begin_frame` 失败、`end_frame` 渲染异常、析构时有活跃帧。
- 可选 GPU smoke test：创建窗口/设备，绘制一个空 ImGui 窗口，截图或检查无错误。

验收：

- 空 UI 帧可以渲染到交换链；`ImGui::NewFrame()` 和 `ImGui::Render()` 各一次。
- 默认字体加载成功；设备对象在析构时释放；没有泄漏/崩溃。
- headless 模式下生成 draw data，测试不依赖 GPU。
- 设备/窗口缺失时的错误信息清晰。
- `ctest` 全部通过；GPU 测试默认跳过。

风险与缓解：

- Diligent 自带的 ImGui 版本是唯一版本：只允许一份 ImGui；在 CMake 和 README 中明确。
- ImGui 1.92 动态字体纹理路径需要 `ImGuiBackendFlags_RendererHasTextures`；如果
  `ImGuiDiligentRenderer` 不支持，退化为显式 `Fonts->Build()` 并手动创建纹理。
- headless texture stub 只是测试设施，不能演化成第二个渲染后端。

### 8.4 M2 — 通用层、panel 与 runtime 适配

**目标：** 实现“多层共享一帧”的核心模型，并接入 runtime；层只支持 order/visible。

交付物：

- `ui::layer`：
  - name/order/visible；
  - 根 `ui::layout`；
  - 层内 panel 通过布局树添加；
  - 受管固定矩形窗口创建和 flags 映射；
  - panel 异常时 `ImGui::End()` + 帧清理。
- `ui::layout` 的树结构和 placement 骨架：方向、gap、padding、添加 panel/子 layout；
  M2 只保证基础分配（固定 basis/均分），完整 grow/shrink/min/max/反向/嵌套语义在 M3。
- `ui::panel` / `panel_context`：内容契约、`panel_feature`。
- `ui::system`：
  - 层栈、稳定排序缓存、可见性；
  - `has_visible_content()`；
  - `draw()` 遍历层、计算布局 placement、绘制 panel；
  - `wants_mouse()` / `wants_keyboard()` 全局捕获；
  - `begin_frame` / `draw` / `end_frame` 配对和异常语义。
- `ui::frame_stage`：
  - 继承 `runtime::frame_stage`，variable-step，`on_update` 为空；
  - `on_render` 检查可见内容、调用 `device.begin_frame`、`system.begin_frame/draw/end_frame`；
  - `frame_provider` 测试 seam：允许注入自定义 `frame_context`，便于无 GPU 测试和离屏目标。
- z-order 保护的第一版（内部 API 或降级约定，M2 评审确认）。

验收：

- 两个通用层（例如 `layer_a` / `layer_b`）在同一个 ImGui 帧中绘制；隐藏层完全跳过。
- 一个 `frame_stage` 的 `on_render` 只产生一次 `NewFrame` / `Render`。
- 层内布局能生成 panel 的显式矩形；panel 不再自己设置位置/尺寸。
- panel 抛异常后 ImGui 不处于“帧进行中”状态，异常传播出 `step_frame`。
- 层顺序、同 order 稳定性、删除 panel/层后的指针失效规则有测试。
- headless 测试通过；GPU 测试可选。

风险与缓解：

- z-order：见 6.3；M2 必须做决定并写测试/文档，不能留到后期。
- `frame_stage` 重复调用 `device.begin_frame`：render-core 明确支持同一 presented frame
  多次 begin_frame（只重绑目标，不推进 frame index）；在文档和测试中固定这一点。如果未来
  render-core 改变语义，改用 `frame_provider` 共享 frame context。
- 面板窗口 ID 冲突：统一使用 `###` 稳定 ID 和 `PushID`，测试动态标题场景；应用负责层内
  panel id 唯一。

### 8.5 M3 — sandbox 迁移（本次不含 flex 布局）

> 实现说明：本次 M3 只做 sandbox 迁移、删除 legacy 和文档更新；`ui::layout` flex 相关
> 交付物顺延到后续里程碑。panel 窗口固定填满层 viewport，内容使用 ImGui 原生定位。

**目标：** 把 sandbox-demo 迁移到分层 UI 模型。

交付物：

- `ui::layout`：
  - `flex_direction`：row / row_reverse / column / column_reverse；
  - `flex`：grow / shrink / basis / min_width / min_height / max_width / max_height；
  - 嵌套 layout；
  - gap / padding；
  - 隐藏 panel 重排；
  - placement 计算（panel 指针 + rect），不依赖 ImGui 绘制即可测试。
- 布局算法：
  - 初始主轴尺寸 = clamp(basis, min, max)；
  - grow 分配剩余空间，按 max 冻结并重新分配；
  - shrink 按 `shrink * basis` 加权收缩，按 min 冻结并重新分配；
  - 交叉轴拉伸到容器尺寸，按 min/max 限制；
  - 反向方向只改变位置，不改变绘制顺序；
  - 负值/非有限值清洗；
  - min > max 时按 max = min 处理。
- 与 ImGui 集成：`SetNextWindowPos` + `SetNextWindowSize` 固定矩形；panel 内容溢出由
  panel 自己滚动/裁剪。
- sandbox-demo 迁移：
  - 删除 `sandbox_imgui_host` 和 legacy 头；
  - 创建 `ui::system`，迁移字体候选、style、输入设置；
  - `hud_panel` / `pause_menu_panel` 改成新 `ui::panel`（只画内容，不再设置窗口位置）；
  - 应用**自己定义** HUD/menu 层（`ui::layer` 只是 name/order/visible；库不知道它们
    是 HUD/menu）；
  - HUD 层使用 flex 根布局（padding/gap/cross stretch）；
  - menu 层由应用在打开时显示、关闭时隐藏；应用自己隐藏 HUD 层、阻止游戏输入；
  - 注册一个 `ui::frame_stage`，render order 高于世界 stage；
  - 删除两个旧的 `ui::frame_stage` runtime stage；
  - 更新 `session.ui_wants_mouse` 为 `ui.wants_mouse()`；
  - 保留 ESC 开关和中文/英文回退行为。
- 更新 `sandbox-demo/AGENTS.md` / `README.md`：ui 只提供分层渲染；HUD/menu 是应用概念；
  只有一个 `ui::frame_stage`。
- 删除 legacy API、旧测试和旧文档描述。

验收：

- sandbox 的 HUD 通过 flex 布局定位；menu 由应用显示/隐藏，打开时 HUD 隐藏、游戏输入被
  阻止；库中没有 HUD/menu 类型或预设。
- 窗口 resize 后 flex 布局重新计算；panel 矩形随容器变化。
- 只有一个 UI runtime stage；帧内只有一次 `ImGui::NewFrame` / `Render`（日志或测试断言）。
- sandbox 构建/测试通过；截图与迁移前视觉基本一致。
- 旧 `ui::host`、单 panel 的 `ui::frame_stage` 和 legacy 头不再存在；新的
  `ui::layer` 是层类型。
- 布局单元测试覆盖：
  - row/column 和两个 reverse；
  - grow 1:1、1:2、多子项；
  - basis 固定 + 剩余 grow；
  - max 冻结和再次分配；
  - shrink、min 冻结和溢出；
  - min/max 双轴；
  - 交叉轴拉伸和 max 截断；
  - gap、padding、嵌套 layout；
  - 隐藏 panel 重排；
  - 零/负/非有限输入。

风险与缓解：

- flex 的 grow/shrink + min/max 冻结算法容易出错：M3 用表驱动测试和随机化约束测试固定
  行为；算法保持单趟/冻结迭代的简单实现，不引入完整 flexbox 的复杂特性。
- 固定矩形意味着 panel 内容不能撑高窗口：文档要求 panel 自己滚动/裁剪；sandbox 的 HUD
  需要检查内容是否溢出。
- 迁移期间 HUD/menu 视觉回归：保留截图和手动测试清单；逐个 panel 对比。
- 反向方向与嵌套布局组合：测试覆盖。

### 8.6 M4 — 完整输入桥与全局捕获

**目标：** 让 UI 真正可交互，并把全局捕获语义固定下来。

交付物：

- 完整 `key_id` → `ImGuiKey` 表：字母、数字、标点、功能键、方向键、编辑键、小键盘、
  左右修饰键、菜单键；穷举测试每个映射。
- 鼠标按键 0–4、滚轮、鼠标位置；文本 UTF-8 解码；`release_all` / 焦点丢失的 key-up 补发。
- `scroll_scale`、`keyboard_nav` 配置；手柄导航留到后续（platform 当前没有手柄输入）。
- `wants_mouse()` / `wants_keyboard()` 的时序和文档；不引入层级的 modal/capture 策略。
- headless 输入测试：注入事件、断言 IO 状态、`WantCaptureMouse/Keyboard`、文本输入、滚轮、
  修饰键、失焦释放。
- 光标策略：v1 由应用处理；文档给出显示/隐藏光标的示例。

验收：

- ImGui 文本输入框可以输入 ASCII 和中日韩文本（使用 M5 的 CJK 字体；M4 可用默认字体验证
  ASCII）。
- 应用可以通过隐藏层或给 panel 加 `no_input` 实现自己的 modal 行为；库不参与该决策。
- 窗口失焦后没有卡住的按键；重新聚焦后输入正常。
- 输入映射表覆盖所有 `key_id` 中列出的键；未映射键有明确行为。

风险与缓解：

- 键位映射遗漏：用编译期/运行时表测试覆盖；新增 `key_id` 时测试失败提醒更新。
- 滚轮单位不一致：`scroll_scale` 可配置，文档说明平台默认值。
- 捕获状态一帧延迟：文档说明；应用需要更严格时在 `on_frame_begin` 读取上一帧状态，或用
  自己的 UI 状态阻止游戏输入。
- IME/剪贴板不做；需要时作为独立里程碑。

### 8.7 M5 — 字体、CJK、DPI 与主题

**目标：** 达到可发布的中文/高 DPI 体验。

交付物：

- `font_config` 完整实现：默认字体、CJK 回退、custom glyph ranges、merge、reload。
- `ui::find_system_cjk_font()` 和 `font_loaded(id)`；sandbox 恢复“中文优先、英文回退”。
- DPI 策略落地并用截图验证：
  - macOS Retina 2x；
  - Windows 150% / 200%（至少手动验证）；
  - 字体清晰、布局尺寸一致、鼠标命中正确。
- 主题：dark/light 预设；`system.style()` 自定义；sandbox 蓝色主题通过 style hook 保留。
- 不提供 diagnostics 面板或统计 UI。

验收：

- 中文 HUD/menu 在 1x/2x 下清晰；CJK 字体缺失时自动回退英文，不崩溃。
- `reload_fonts()` 后新字体生效，旧纹理释放。
- 主题切换不破坏布局；sandbox 主题与迁移前一致。
- 字体图集大小和加载时间在可接受范围（记录基线）。

风险与缓解：

- CJK 常用字范围图集较大：使用动态纹理/按需增长；记录内存和首帧时间；必要时提供按需
  glyph 范围配置。
- 系统字体不可用/许可问题：应用自带字体优先；`find_system_cjk_font()` 只做辅助。
- DPI 组合在 ImGui 版本间行为不同：用截图和鼠标命中测试固定；必要时改为 framebuffer 像素
  坐标系，但保持应用侧逻辑点 API 不变。

### 8.8 M6 — 健壮性、测试与发布

**目标：** 把 v0.3 从“可用”推进到“可依赖”。

交付物：

- 设备丢失/重置接口：`system::on_device_lost()` / `on_device_reset()`；renderer 的
  invalidate/create；字体纹理重建。
- 交换链 resize、窗口最小化、零尺寸、DPI 变化路径。
- 异常安全审计：panel、字体、renderer、`begin_frame`/`end_frame`、析构。
- 状态恢复契约：scissor/viewport/render targets；UI 之后还有 pass 的示例。
- 测试：
  - flex 布局、层、输入、system 生命周期的完整单元测试；
  - headless ImGui 集成测试；
  - 可选 GPU 截图测试（`CYBERVERSE_UI_TEST_GPU=1`）；
  - 性能基准：1/10/100 panel 的布局和 draw data 生成；字体 reload。
- 文档：
  - README 重写：定位、固定依赖、生命周期、帧流程、flex 布局限制、迁移指南；
  - API 头 Doxygen；
  - `ui/CLAUDE.md` 最终规则；
  - workspace AGENTS.md 最终依赖图。
- 发布：
  - 版本头/CHANGELOG；
  - CMake install/export（如果工作区需要）；
  - 标记 API 为稳定或 experimental；
  - 格式化、警告清理、CI。

验收：

- 所有单元/headless 测试通过；GPU 测试在支持的机器上通过。
- 没有已知的 ImGui 状态泄漏、悬空指针、重复 ImGui 符号。
- sandbox-demo 在 macOS 和至少一个其他平台构建/运行；中文/英文、HUD/menu/flex 布局正常。
- 性能基线记录：UI 帧时间、draw call、字体加载时间。
- README/AGENTS/roadmap 一致，没有旧“backend-agnostic”描述。

风险与缓解：

- 设备丢失在 Diligent/render-core 中没有完整信号：v1 只提供手动接口和测试；自动检测留到
  render-core 支持后。
- 跨平台字体/DPI 差异：截图和手动清单；CI 至少覆盖 headless 测试。
- API 冻结过早：M6 前保持 experimental，收集 sandbox 和未来编辑器的反馈。

---

## 9. 测试策略

| 层级 | 内容 | 依赖 GPU/窗口 | 运行方式 |
| --- | --- | --- | --- |
| 纯单元 | flex 布局、层排序、key map、panel 树 | 否 | 默认 `ctest` |
| Headless ImGui | system 生命周期、单帧、布局 placement、输入事件、异常清理 | 否（Diligent renderer 跳过，测试 texture stub） | 默认 `ctest` |
| Runtime 集成 | `ui::frame_stage` 与 `frame_pipeline::step_frame`，frame_provider 注入 | 否 | 默认 `ctest` |
| GPU smoke | 真实 device/swapchain，绘制 panel，截图/像素检查 | 是 | `CYBERVERSE_UI_TEST_GPU=1`，opt-in |
| 基准 | flex 布局、draw data 生成、字体 reload | 否 | 手动/benchmark target |

测试要求：

- 所有测试在 `/tmp` 或等价外部目录构建运行，不写入仓库。
- 默认测试不打开窗口、不要求 Vulkan/MoltenVK。
- headless 测试使用 `platform` 的 headless 窗口注入事件；不要 sleep，不要依赖真实计时。
- GPU 测试复用 render-core 的可选测试模式，失败时输出清晰的后端/驱动信息。
- flex 测试覆盖：
  - 9 种以上尺寸组合、4 个方向、grow/shrink/basis、min/max、gap/padding、嵌套、隐藏、
    零/负/非有限值；
  - 表驱动期望矩形；
  - 随机化约束测试：任意合法 flex 输入下，子项矩形非负、顺序稳定、不超出容器（除非 min
    导致溢出）。
- 层测试覆盖同 order 稳定性、删除/清空、hidden 跳过、所有层无可见 panel 时跳过整帧。
- 输入测试覆盖所有映射键、文本、滚轮、修饰键、失焦释放、`WantCaptureMouse/Keyboard`。
- 异常测试覆盖 panel 抛异常、字体加载失败、renderer 初始化失败、析构时有活跃帧。
- 不测试 HUD/diagnostics/menu/modal 等应用语义，因为库不提供这些概念。

---

## 10. 迁移计划（sandbox-demo）

### 10.1 M0：保持可构建

- 旧 API 移入 `ui::legacy`；sandbox 暂时改用 legacy 头。
- 不改变 sandbox 行为；只保证构建和测试。

### 10.2 M3：切换到新模型

1. 删除 `sandbox_imgui_host.hpp/.cpp`；其职责拆到 `ui::system`：
   - 字体候选 → `system_config.fonts`；
   - style → `system.style()` 或 `ui::theme`；
   - 输入 → `ui::detail::input_bridge`；
   - Diligent ImGui 渲染 → `system::impl` 内部的 `ImGuiDiligentRenderer`。
2. `hud_panel` / `pause_menu_panel`：
   - 继承新 `ui::panel`；
   - 删除 `ImGui::Begin/End` 和 `SetNextWindowPos`；
   - 在 `draw(panel_context&)` 中只画内容；
   - `visible()` 保持现有业务条件。
3. 应用定义层（库不感知这些名字）：
   - HUD 层：`order = 100`，应用在 menu 打开时 `set_visible(false)`；
   - menu 层：`order = 200`，应用在 menu 打开时 `set_visible(true)`；
   - menu 的 dim 背景由应用添加一个铺满的 background panel 绘制；库没有 layer background；
   - 应用自己阻止游戏输入（检查 `menu_open` 和 `ui.wants_mouse()`）。
4. 每层使用根 flex 布局：
   - HUD：row/column、padding、gap、grow 比例；
   - menu：应用可以用嵌套 layout + spacer 居中，或使用一个全屏 panel 在内容里自行居中。
5. 注册一个 `ui::frame_stage`，render order 高于世界 stage；`receive_update = false`；
   删除原来的 HUD/menu runtime stage。
6. `session.ui_wants_mouse` 改为 `ui.wants_mouse()`。
7. 删除 legacy 头和旧测试；更新 sandbox 的 CMake。
8. 视觉对比：HUD 位置、menu 居中/遮罩、中文/英文回退、ESC 开关、相机输入屏蔽。

> 注意：HUD/menu/遮罩/模态是 **sandbox 应用逻辑**，不是 `ui` 库的功能。库只负责按顺序
> 绘制层和按 flex 计算 panel 矩形。

### 10.3 M5：增强

- 添加 CJK 字体配置和 `font_loaded("cjk")` 回退。
- 保留 sandbox 蓝色主题；如果移入库，则作为可选主题或 style hook。
- 更新 README/AGENTS 截图和说明。

---

## 11. 风险与缓解

| 风险 | 影响 | 缓解 |
| --- | --- | --- |
| flex grow/shrink + min/max 冻结算法实现错误 | 布局重叠/尺寸错误 | 表驱动测试、随机化约束测试、简单冻结迭代；不扩展完整 flexbox |
| 固定矩形导致内容溢出 | 文本/控件被裁剪 | panel 自己滚动/裁剪；文档明确；sandbox 检查 HUD 内容 |
| `Diligent-Imgui` 目标不可用 | 配置失败 | 配置阶段明确报错；要求 `CYBERVERSE_DILIGENT_DIR` / `DILIGENT_BUILD_TOOLS`；不回退其他后端 |
| ImGui/Diligent 版本强耦合 | API 断裂 | 固定依赖版本；pimpl 隔离；升级时只改 `system::impl` |
| 单上下文层 z-order 不完美 | 窗口重叠顺序错误 | `NoBringToFrontOnFocus` + 集中 z-order 保护；应用用可见性/`no_input` 避免重叠交互层 |
| DPI/FontGlobalScale 行为随 ImGui 版本变化 | 字体模糊/尺寸错误 | 逻辑点坐标 + 截图/命中测试；M5 允许调整为 framebuffer 坐标系但保持 API |
| CJK 字体图集过大 | 内存/首帧卡顿 | 动态纹理、按需 glyph 范围、应用自带字体、记录基线 |
| 输入映射不完整 | 快捷键/文本异常 | 穷举 key map 测试；新增 key_id 时测试失败 |
| 捕获状态一帧延迟 | 游戏输入偶尔穿透 | 应用用自己的 UI 状态/层可见性立即阻止；文档说明 |
| panel 异常导致 ImGui 帧卡住 | 后续帧崩溃 | RAII window guard；`EndFrame` 清理；异常路径测试 |
| UI stage 顺序错误 | UI 被世界覆盖/看不到 | runtime render order 约定；文档/测试；UI 作为最后渲染阶段 |
| 破坏性 API 变更影响 sandbox | 主分支不可构建 | M0 legacy shim；M3 同分支迁移并删除；CI 门禁 |
| 固定依赖导致构建环境要求高 | 新开发者配置困难 | README 明确 Diligent/Vulkan 依赖；CMake 错误信息可操作 |

---

## 12. 未决问题

这些问题需要在对应里程碑评审时确认：

1. **层 z-order 是否接受 ImGui 内部 API？**  
   建议：接受，集中在 `detail/z_order.cpp` 并做版本检查；不接受时采用 6.3 的降级约定。
2. **`ui::layer` 与 `ui::frame_stage` 的命名。**  
   已实现：UI 层改名为 `ui::layer`，runtime 适配器改名为 `ui::frame_stage`；避免层类型与
   `runtime::frame_stage` 概念混淆。
3. **panel 是否永远不管理顶层窗口？**  
   建议：v1 是；浮动调试窗口作为后续 `free_panel` 显式能力，不污染 flex 布局。
4. **是否允许应用在 `system.draw()` 之外直接调用 ImGui？**  
   建议：允许在 `begin_frame` / `end_frame` 之间通过显式逃生口调用，但文档标明风险；正常
   路径只用 panel。
5. **离屏 UI / 多 viewport 的时间点？**  
   建议：v1 只做交换链；`frame_provider` 和 `frame_context` 入口为后续离屏保留，多 viewport
   需要单独里程碑。
6. **光标策略是否进入 `layer_desc`？**  
   建议：v1 由应用处理；如果重复出现需求，再增加 cursor policy。
7. **是否要让每个 UI 层都能被 runtime 独立调度？**  
   建议：v1 只保留一个 `ui::frame_stage` 负责渲染；如果层需要不同更新频率，由应用在自己的
   update 阶段推进，渲染仍然集中在单一 adapter。
8. **flex 是否需要百分比 basis / `auto` 内容尺寸 / justify-content / align-items？**  
   建议：v1 不做。当前双轴拉伸 + grow/shrink/basis 已覆盖核心需求；需要时用嵌套 layout 和
   spacer，或作为后续独立里程碑扩展。
9. **是否需要 layout 级别的 `visible`？**  
   建议：v1 不做；应用通过清除子树或把 `grow` 设为 0 处理。若出现频繁需求，再增加
   `layout::set_visible()`。

---

## 13. 完成定义（Definition of Done）

v0.3 完成时，应满足：

- [ ] `ui` 固定依赖 Dear ImGui + DiligentEngine；没有后端接口、后端选择或替代渲染路径。
- [ ] `ui` 拥有 ImGui 上下文、Diligent ImGui 渲染器、输入桥、字体、主题和 DPI 处理。
- [ ] 一个引擎帧只开启一次 ImGui 帧；所有层共享一次 `NewFrame` / `Render`。
- [ ] 层只提供 name/order/visible/root layout；库没有 HUD/diagnostics/menu 类型、预设或
      诊断面板。
- [ ] 唯一的 `ui::layout` 支持 flex-direction、grow/shrink/basis、子项 min/max、gap、
      padding、嵌套和 XY 双轴拉伸。
- [ ] 所有 panel 都获得显式矩形，不使用内容自适应尺寸。
- [ ] `ui::frame_stage` 是唯一 runtime 适配器；应用不再实现 ImGui host。
- [ ] CJK 字体/英文回退和 Retina/高 DPI 渲染验证通过。
- [ ] sandbox-demo 完成迁移，使用应用自定义的层和 flex 布局；视觉和行为与迁移前一致或更好。
- [ ] 旧 `ui::host`、单 panel `ui::layer`、legacy 头和“backend-agnostic”文档全部删除。
- [ ] 单元/headless 测试全绿；可选 GPU 测试通过；性能基线记录。
- [ ] README、API 注释、`ui/CLAUDE.md`、工作区 AGENTS.md 与实现一致。
- [ ] 没有重复 ImGui 符号、隐藏线程、未定义的帧内状态或悬空 panel 指针。

---

## 14. 附录

### 14.1 应用侧示例（目标 API）

```cpp
#include "cyberverse/ui.hpp"
#include "cyberverse/ui/imgui.hpp"

class status_panel final : public cyberverse::ui::panel {
public:
  explicit status_panel(const app_status& status) : status_{ &status } {}

  [[nodiscard]] std::string_view id() const noexcept override { return "app.status"; }
  [[nodiscard]] bool visible() const noexcept override { return true; }
  [[nodiscard]] cyberverse::ui::panel_feature features() const noexcept override {
    return cyberverse::ui::panel_feature::no_input; // 只显示，不接收输入
  }

  void draw(cyberverse::ui::panel_context&) override {
    ImGui::Text("FPS %.0f", status_->fps);
    ImGui::Text("Objects %zu", status_->object_count);
  }

private:
  const app_status* status_ = nullptr;
};

// ui 由应用成员持有，并保证比 runtime stage 长寿。
// 初始化示例：
//   ui::system_config config;
//   config.device = &device;
//   config.window = &window;
//   config.fonts = {
//     ui::font_config{ .id = "default", .size = 16.0f },
//     ui::font_config{ .id = "cjk", .path = ui::find_system_cjk_font(), .size = 16.0f,
//                      .range = ui::glyph_range::chinese_simplified_common, .merge = true },
//   };
//   auto ui = ui::system::create(config);
//   setup_ui(*ui, device, pipeline, ui_clock, status, session);
void setup_ui(cyberverse::ui::system& ui_system,
              cyberverse::render_core::render_device& device,
              cyberverse::runtime::frame_pipeline& pipeline,
              cyberverse::runtime::clock_node& ui_clock,
              const app_status& status,
              const app_session& session) {
  // 应用自定义的底层：库只知道它是一层 UI。
  auto status_layer = std::make_unique<cyberverse::ui::layer>(
      cyberverse::ui::layer_desc{
          .name = "app.status",
          .order = 100,
      });

  auto& status_root = status_layer->root_layout();
  status_root.set_direction(cyberverse::ui::flex_direction::row);
  status_root.set_gap(12.0f);
  status_root.set_padding(cyberverse::ui::insets{ 16.0f, 16.0f, 16.0f, 16.0f });
  status_root.add(std::make_unique<status_panel>(status),
                  cyberverse::ui::flex{
                      .grow = 1.0f,
                      .shrink = 1.0f,
                      .basis = 0.0f,
                      .max_width = 420.0f,
                  });
  status_layer->set_visible(!session.settings_open); // 应用逻辑，不是库的 modal 策略
  ui_system.add_layer(std::move(status_layer));

  // 应用自定义的上层；库不感知“settings”语义。
  auto settings_layer = std::make_unique<cyberverse::ui::layer>(
      cyberverse::ui::layer_desc{
          .name = "app.settings",
          .order = 200,
      });

  auto& settings_root = settings_layer->root_layout();
  settings_root.set_direction(cyberverse::ui::flex_direction::column);
  settings_root.set_gap(8.0f);
  settings_root.set_padding(cyberverse::ui::insets{ 24.0f, 24.0f, 24.0f, 24.0f });

  // 上下 spacer 把中间的 row 垂直居中；row 内的左右 spacer 把 panel 水平居中。
  auto& top_spacer = settings_root.add_layout(cyberverse::ui::flex{ .grow = 1.0f, .basis = 0.0f });
  (void)top_spacer;

  auto& center_row = settings_root.add_layout(cyberverse::ui::flex{
      .grow = 0.0f,
      .shrink = 0.0f,
      .basis = 520.0f,
  });
  center_row.set_direction(cyberverse::ui::flex_direction::row);

  auto& left_spacer = center_row.add_layout(cyberverse::ui::flex{ .grow = 1.0f, .basis = 0.0f });
  (void)left_spacer;
  center_row.add(std::make_unique<settings_panel>(session),
                 cyberverse::ui::flex{
                     .grow = 0.0f,
                     .shrink = 0.0f,
                     .basis = 420.0f,
                 });
  auto& right_spacer = center_row.add_layout(cyberverse::ui::flex{ .grow = 1.0f, .basis = 0.0f });
  (void)right_spacer;

  auto& bottom_spacer = settings_root.add_layout(cyberverse::ui::flex{ .grow = 1.0f, .basis = 0.0f });
  (void)bottom_spacer;

  settings_layer->set_visible(session.settings_open);
  ui_system.add_layer(std::move(settings_layer));

  auto ui_stage = std::make_unique<cyberverse::ui::frame_stage>(ui_system, device);
  cyberverse::runtime::frame_stage_options options;
  options.render_order = 1000;
  options.receive_update = false;
  options.receive_render = true;
  pipeline.add_stage("ui", std::move(ui_stage), ui_clock, options);

  // 应用需要在游戏输入前查询：
  //   ui_system.wants_mouse()
  //   ui_system.wants_keyboard()
}
```

### 14.2 术语

| 术语 | 含义 |
| --- | --- |
| UI 层 / layer | `ui::layer`，只有 name/order/visible/root layout。 |
| panel | 层内一个受管固定矩形窗口的内容单元，实现 `ui::panel`。 |
| layout | 唯一的 flex 容器；可嵌套，拥有 panel 和子 layout。 |
| flex 子项 | panel 或嵌套 layout 在父布局中的 grow/shrink/basis/min/max 配置。 |
| grow | 主轴剩余空间的分配比例。 |
| shrink | 主轴溢出时的收缩权重。 |
| basis | 主轴初始尺寸（逻辑点）。 |
| 双轴拉伸 | 主轴由 flex 填满，交叉轴默认拉伸到容器尺寸。 |
| placement | 布局计算出的 panel 指针 + 显式矩形。 |
| overlay pass | UI 在 3D 之后提交的一次 ImGui draw data 渲染。 |
| headless | 无 GPU/窗口的测试模式，跳过 Diligent 绘制但保留 ImGui 布局/输入。 |

### 14.3 参考

- [Dear ImGui](https://github.com/ocornut/imgui)
- [DiligentEngine ImGui integration](https://github.com/DiligentGraphics/DiligentEngine/tree/master/DiligentTools/Imgui)
- [render-core 的 frame_context / render_device](../../render-core/README.md)
- [runtime 的 frame_stage / frame_pipeline](../../runtime/docs/roadmap.md)
- [sandbox-demo 的 UI 迁移](../../sandbox-demo/AGENTS.md)
