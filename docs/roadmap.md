# Cyberverse UI 路线图

## 1. 当前版本：v0.6

- **定位：** 固定 Dear ImGui + DiligentEngine 的分层 UI 运行时，加一套函数式
  widgets 组件层。
- **硬依赖：** `runtime`、`platform`、`render-core`、Dear ImGui、
  DiligentEngine、`animation`（仅 widgets 属性动画）。
- **不依赖 Yoga。** 旧即时模式 widgets 树和旧 `panel` 容器已经移除。
- **没有独立 layer。** `system` 直接管理有序 window 列表。

```text
ui::system
  ├─ window A (order 400)
  ├─ window B (order 300)
  └─ window C (order 200)
       └─ application draw(window_context&)
            └─ ui::widgets::run(root, {dt, theme, animator})
                 └─ widgets::state + component<Impl>
```

## 2. 核心分层

### 2.1 `ui::system`

拥有：

- 唯一 Dear ImGui context；
- Diligent ImGui renderer；
- 平台输入桥；
- 字体与 ImGui 主题；
- 有序 window 列表；
- 每帧的 ImGui `NewFrame`/`Render` 生命周期；
- window 名称、feature flags、局部矩形和 z-order。

一个引擎帧最多打开一个 ImGui 帧。非 headless 帧绑定 color/depth target 并重置
scissor。

### 2.2 `ui::window`

顶层窗口宿主，不是 widget。

- `id`、`title`、`order`、`visible`、`features`、`window_rect`、
  `draw(window_context&)`；
- 默认填满 system viewport；
- `window_rect` 可请求局部窗口，system 负责 clamp；
- `order()` 数值越大越靠上，相同 order 保持插入顺序；
- feature：title bar、scrollbar、transparent、no input、focus on appearing、
  bring to front on focus；
- 输入捕获、z-order、异常清理由 system 统一处理；
- 窗口内容可以用原生 ImGui，也可以调用 `widgets::run`。

window id 在 system 内全局唯一；ImGui 窗口名为 `title###id`。

### 2.3 公共几何类型

`ui::rect` 和 `ui::layout_size` 位于 `src/cyberverse/ui/rect.hpp`：

- window / window_context 使用同一个 `ui::rect`；
- widgets 通过 `widgets::rect` 别名使用同一个类型；
- 测量和布局使用 `ui::layout_size`（width/height），不再使用容易混淆的
  `widgets::size`。

### 2.4 `ui::widgets`

函数式组件层：

- 持久数据全部在调用者持有的 `widgets::state`；
- 工厂返回静态 `component<Impl>`；
- 容器保存命名子组件引用；
- 无虚函数、无类型擦除、无 `std::vector` 子项；
- 唯一 `std::function` 是 state 事件回调；
- 每帧入口：

```cpp
widgets::run(root, {dt, theme, animator});
```

固定顺序：

```text
tick 输入属性 → measure → 设置 bounds → draw
```

`measure()` 返回 `layout_size`；`state::bounds()` 使用共享 `ui::rect`。

## 3. 组件

| 组件 | 说明 |
| --- | --- |
| `text` | 文本，支持字号、颜色、换行符 |
| `button` | 原生 ImGui 按钮，点击发 `click` |
| `checkbox` / `radio` | 外部 bool/int，值变化发 `value_changed` |
| `text_input` | 外部 `std::string`，原生单行输入 |
| `divider` | 水平/垂直分隔线 |
| `stack` | 编译期固定子项的一维排布 |
| `flexbox` | no-wrap row/column、gap/padding、justify/align、grow/shrink/basis |
| `scroll_view` | 双轴滚动、滚轮和双滚动条、静态子项 |
| `box` | 带背景/边框/圆角的单子项内容容器 |

不支持：

- collapse_group；
- native escape hatch；
- 运行时动态子项/虚拟列表；
- flex-wrap；
- 布局结果 FLIP 动画。

## 4. 尺寸与动画

- `width`、`height`、`opacity`、`font_size` 是 float property；
- flex 子项还读取 `flex_grow`、`flex_shrink`、`flex_basis`；
- 滚动偏移是 `scroll_x`、`scroll_y`；
- 动画只作用于布局输入，布局结果由每次 measure/layout 重新计算；
- 显式 width 或 height 为 0 时，跳过子项 measure/draw；
- 任意轴 bounds 为 0 时跳过绘制；
- 零尺寸子项不参与容器排布和 gap。

## 5. 帧流程

1. `frame_stage::on_render` 检查 `system::has_visible_content()`；
2. 获取 `render_core::frame_context`；
3. `system::begin_frame(frame, dt)`：
   - 绑定 target、重置 scissor；
   - 输入送入 ImGui；
   - `ImGui::NewFrame()`；
4. `system::draw()`：
   - 按 window `order()` 升序遍历；
   - 跳过隐藏 window；
   - 创建受管 ImGui window；
   - 计算 viewport/local rect；
   - 调用 `window::draw(window_context&)`；
   - 按绘制顺序 raise；
5. `system::end_frame()`：
   - `ImGui::Render()`；
   - 一次 `RenderDrawData()`；
   - 记录 mouse/keyboard capture。

窗口异常必须结束窗口并丢弃当前 ImGui 帧；下一帧仍必须可用。

## 6. 迁移里程碑

### M0 — 固定分层运行时（完成）

- system/window/frame_stage；
- 字体、IME、输入桥、z-order。

### M1 — 函数式 widgets（完成）

- state/component/concept；
- text/button/stack；
- 属性动画、visibility、opacity、font_size。

### M2 — 布局与控件（完成）

- flexbox；
- checkbox/radio/text_input/divider；
- 双轴 scroll_view；
- box；
- 零尺寸规则。

### M3 — 单一 widgets 层（完成）

- 删除旧 Yoga widgets；
- 删除旧 `widgets::panel`；
- `ui::panel` → `ui::window`；
- widgets2 重命名为 widgets；
- sandbox/diagnosis 迁移；
- 删除 Yoga 依赖和 CMake 模块。

### M4 — 合并 window/layer（完成）

- 删除 `ui::layer`；
- system 直接拥有 window；
- layer 的 order/visibility 合并进 window；
- window id 改为 system 全局唯一；
- z-order 改为 window order + 插入顺序；
- sandbox/diagnosis 改为逐 window 可见性。

### M5 — 稳定性与文档（进行中）

- headless 交互测试；
- GPU smoke；
- sandbox/diagnosis 回归；
- 文档和示例统一到 window + widgets。

### M6 — 文本与 IME 完善（待做）

- macOS inline preedit/candidate 定位；
- IME composition 渲染；
- 文本选择/剪贴板一致性；
- 多行文本与自动换行。

## 7. 非目标

- 不重新引入 Yoga、第二套布局引擎或第二套 widgets；
- 不重新引入 layer 或 window-group 中间层；
- 不实现动态子项列表；
- 不实现 collapse_group/native；
- 不实现 flex-wrap；
- 不引入后端抽象或其他 UI 渲染后端；
- 不增加 HUD/menu/diagnostics 库类型。
