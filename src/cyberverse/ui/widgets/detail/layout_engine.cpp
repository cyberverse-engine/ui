/**
 * @file layout_engine.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Yoga-backed widget layout and draw scheduling
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/detail/layout_engine.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <unordered_map>
#include <utility>
#include <vector>

#include <yoga/Yoga.h>

#include "cyberverse/ui/widgets/detail/draw_utils.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

/**
 * @brief Detail-only bridge to the private widget layout/draw interface
 *
 */
struct layout_access {
  static measured_size measure(widget& owner, measure_context& context, const constraints& limits) {
    return owner.measure(context, limits);
  }

  static void draw(widget& owner, draw_context& context) { owner.draw(context); }

  static void visit_children(const widget& owner, const widget::child_visitor& visitor) {
    owner.visit_children(visitor);
  }

  static bool is_measure_leaf(const widget& owner) noexcept { return owner.is_measure_leaf(); }
  static bool clips_children(const widget& owner) noexcept { return owner.clips_children(); }
  static flex_direction flow_direction(const widget& owner) noexcept { return owner.flow_direction(); }
  static insets flow_padding(const widget& owner) noexcept { return owner.flow_padding(); }
  static float flow_gap(const widget& owner) noexcept { return owner.flow_gap(); }
  static justify_content flow_justify(const widget& owner) noexcept { return owner.flow_justify(); }
  static align_items flow_align(const widget& owner) noexcept { return owner.flow_align(); }

  static const size_spec& width_spec(const widget& owner) noexcept { return owner.width_; }
  static const size_spec& height_spec(const widget& owner) noexcept { return owner.height_; }
  static const std::optional<float>& grow(const widget& owner) noexcept { return owner.grow_; }
  static const std::optional<float>& shrink(const widget& owner) noexcept { return owner.shrink_; }
  static insets margin(const widget& owner) noexcept { return owner.margin_; }
  static float min_width(const widget& owner) noexcept { return owner.min_width_; }
  static float max_width(const widget& owner) noexcept { return owner.max_width_; }
  static float min_height(const widget& owner) noexcept { return owner.min_height_; }
  static float max_height(const widget& owner) noexcept { return owner.max_height_; }
  static std::string_view id(const widget& owner) noexcept { return owner.id_; }

  static void set_bounds(widget& owner, const rect& value) noexcept { owner.bounds_ = value; }
};

namespace {

constexpr float k_max_dimension = 1.0e7f;

[[nodiscard]] float finite_non_negative(float value) noexcept {
  return (std::isfinite(value) && value > 0.0f) ? value : 0.0f;
}

[[nodiscard]] float finite_or(float value, float fallback) noexcept { return std::isfinite(value) ? value : fallback; }

[[nodiscard]] measure_mode to_measure_mode(YGMeasureMode mode) noexcept {
  switch (mode) {
  case YGMeasureModeExactly:
    return measure_mode::exactly;
  case YGMeasureModeAtMost:
    return measure_mode::at_most;
  case YGMeasureModeUndefined:
  default:
    return measure_mode::undefined;
  }
}

[[nodiscard]] bool is_row(flex_direction direction) noexcept {
  return direction == flex_direction::row || direction == flex_direction::row_reverse;
}

[[nodiscard]] YGFlexDirection to_yoga(flex_direction direction) noexcept {
  switch (direction) {
  case flex_direction::row:
    return YGFlexDirectionRow;
  case flex_direction::row_reverse:
    return YGFlexDirectionRowReverse;
  case flex_direction::column_reverse:
    return YGFlexDirectionColumnReverse;
  case flex_direction::column:
  default:
    return YGFlexDirectionColumn;
  }
}

[[nodiscard]] YGJustify to_yoga(justify_content justify) noexcept {
  switch (justify) {
  case justify_content::center:
    return YGJustifyCenter;
  case justify_content::end:
    return YGJustifyFlexEnd;
  case justify_content::space_between:
    return YGJustifySpaceBetween;
  case justify_content::start:
  default:
    return YGJustifyFlexStart;
  }
}

[[nodiscard]] YGAlign to_yoga(align_items align) noexcept {
  switch (align) {
  case align_items::center:
    return YGAlignCenter;
  case align_items::end:
    return YGAlignFlexEnd;
  case align_items::stretch:
    return YGAlignStretch;
  case align_items::start:
  default:
    return YGAlignFlexStart;
  }
}

struct node_payload {
  widget* owner = nullptr;
  const theme* appearance = nullptr;
};

[[nodiscard]] YGSize
measure_callback(YGNodeConstRef node, float width, YGMeasureMode width_mode, float height, YGMeasureMode height_mode) {
  auto* payload = static_cast<node_payload*>(YGNodeGetContext(node));
  if (payload == nullptr || payload->owner == nullptr) {
    return YGSize{ 0.0f, 0.0f };
  }

  constraints limits;
  limits.available_width = finite_non_negative(width);
  limits.available_height = finite_non_negative(height);
  limits.width_mode = to_measure_mode(width_mode);
  limits.height_mode = to_measure_mode(height_mode);

  measure_context context{ payload->appearance };
  const measured_size measured = layout_access::measure(*payload->owner, context, limits);
  return YGSize{ finite_non_negative(measured.width), finite_non_negative(measured.height) };
}

class tree_builder {
public:
  tree_builder(ImVec2 available, const theme& appearance)
      : available_{ finite_non_negative(available.x), finite_non_negative(available.y) }
      , appearance_{ appearance }
      , config_{ YGConfigNew() } {
    YGConfigSetUseWebDefaults(config_, false);
  }

  ~tree_builder() {
    if (root_ != nullptr) {
      YGNodeFreeRecursive(root_);
    }
    if (config_ != nullptr) {
      YGConfigFree(config_);
    }
  }

  tree_builder(const tree_builder&) = delete;
  tree_builder& operator=(const tree_builder&) = delete;

  YGNodeRef build(widget& root) {
    root_ = build_node(root, true, flex_direction::column);
    return root_;
  }

  [[nodiscard]] YGNodeRef node(const widget& owner) const {
    const auto iterator = nodes_.find(&owner);
    return iterator == nodes_.end() ? nullptr : iterator->second;
  }

private:
  YGNodeRef build_node(widget& owner, bool root, flex_direction parent_direction) {
    YGNodeRef node = YGNodeNewWithConfig(config_);
    nodes_.emplace(&owner, node);
    payloads_.push_back(std::make_unique<node_payload>(node_payload{ &owner, &appearance_ }));
    YGNodeSetContext(node, payloads_.back().get());

    if (root) {
      apply_root_size(owner, node);
    } else {
      apply_child_size(owner, node, parent_direction);
    }

    apply_limits(owner, node);
    if (!root) {
      apply_margin(owner, node);
    }

    if (layout_access::is_measure_leaf(owner)) {
      YGNodeSetMeasureFunc(node, &measure_callback);
    } else {
      apply_flow(owner, node);
      int index = 0;
      layout_access::visit_children(owner, [&](widget& child) {
        YGNodeRef child_node = build_node(child, false, layout_access::flow_direction(owner));
        YGNodeInsertChild(node, child_node, index);
        ++index;
      });
    }

    return node;
  }

  void apply_root_size(const widget& owner, YGNodeRef node) const {
    const size_spec width = layout_access::width_spec(owner);
    const size_spec height = layout_access::height_spec(owner);

    const auto apply = [](YGNodeRef target, const size_spec& spec, float available, bool horizontal) {
      switch (spec.mode) {
      case size_mode::fixed:
        if (horizontal) {
          YGNodeStyleSetWidth(target, finite_non_negative(spec.value));
        } else {
          YGNodeStyleSetHeight(target, finite_non_negative(spec.value));
        }
        break;
      case size_mode::stretch:
        if (horizontal) {
          YGNodeStyleSetWidth(target, available);
        } else {
          YGNodeStyleSetHeight(target, available);
        }
        break;
      case size_mode::fit_content:
      default:
        break;
      }
    };

    apply(node, width, available_.x, true);
    apply(node, height, available_.y, false);
  }

  void apply_child_size(const widget& owner, YGNodeRef node, flex_direction parent_direction) const {
    const bool row = is_row(parent_direction);
    const size_spec& width = layout_access::width_spec(owner);
    const size_spec& height = layout_access::height_spec(owner);
    const size_spec& main = row ? width : height;
    const size_spec& cross = row ? height : width;
    const std::optional<float>& explicit_grow = layout_access::grow(owner);
    const std::optional<float>& explicit_shrink = layout_access::shrink(owner);

    const auto set_exact = [&](const size_spec& spec, bool horizontal) {
      if (spec.mode != size_mode::fixed) {
        return;
      }
      if (horizontal) {
        YGNodeStyleSetWidth(node, finite_non_negative(spec.value));
      } else {
        YGNodeStyleSetHeight(node, finite_non_negative(spec.value));
      }
    };

    set_exact(width, true);
    set_exact(height, false);

    const float shrink_default = main.mode == size_mode::fixed ? 0.0f : 1.0f;
    float grow_value = explicit_grow.value_or(main.mode == size_mode::stretch ? 1.0f : 0.0f);
    const float shrink_value = explicit_shrink.value_or(shrink_default);

    YGNodeStyleSetFlexGrow(node, finite_or(grow_value, 0.0f));
    YGNodeStyleSetFlexShrink(node, finite_or(shrink_value, 1.0f));

    if (main.mode == size_mode::stretch) {
      YGNodeStyleSetFlexBasis(node, 0.0f);
    }

    if (cross.mode == size_mode::stretch) {
      YGNodeStyleSetAlignSelf(node, YGAlignStretch);
    }
  }

  void apply_limits(const widget& owner, YGNodeRef node) const {
    const float min_width = finite_or(layout_access::min_width(owner), 0.0f);
    const float min_height = finite_or(layout_access::min_height(owner), 0.0f);
    const float max_width = finite_or(layout_access::max_width(owner), k_max_dimension);
    const float max_height = finite_or(layout_access::max_height(owner), k_max_dimension);

    YGNodeStyleSetMinWidth(node, std::clamp(min_width, 0.0f, k_max_dimension));
    YGNodeStyleSetMinHeight(node, std::clamp(min_height, 0.0f, k_max_dimension));
    YGNodeStyleSetMaxWidth(node, std::clamp(max_width, min_width, k_max_dimension));
    YGNodeStyleSetMaxHeight(node, std::clamp(max_height, min_height, k_max_dimension));
  }

  void apply_margin(const widget& owner, YGNodeRef node) const {
    const insets margin = layout_access::margin(owner);
    YGNodeStyleSetMargin(node, YGEdgeLeft, finite_or(margin.left, 0.0f));
    YGNodeStyleSetMargin(node, YGEdgeTop, finite_or(margin.top, 0.0f));
    YGNodeStyleSetMargin(node, YGEdgeRight, finite_or(margin.right, 0.0f));
    YGNodeStyleSetMargin(node, YGEdgeBottom, finite_or(margin.bottom, 0.0f));
  }

  void apply_flow(const widget& owner, YGNodeRef node) const {
    const insets padding = layout_access::flow_padding(owner);
    const float gap = finite_non_negative(layout_access::flow_gap(owner));

    YGNodeStyleSetFlexDirection(node, to_yoga(layout_access::flow_direction(owner)));
    YGNodeStyleSetJustifyContent(node, to_yoga(layout_access::flow_justify(owner)));
    YGNodeStyleSetAlignItems(node, to_yoga(layout_access::flow_align(owner)));

    YGNodeStyleSetPadding(node, YGEdgeLeft, finite_non_negative(padding.left));
    YGNodeStyleSetPadding(node, YGEdgeTop, finite_non_negative(padding.top));
    YGNodeStyleSetPadding(node, YGEdgeRight, finite_non_negative(padding.right));
    YGNodeStyleSetPadding(node, YGEdgeBottom, finite_non_negative(padding.bottom));
    YGNodeStyleSetGap(node, YGGutterAll, gap);
  }

  ImVec2 available_;
  const theme& appearance_;
  YGConfigRef config_ = nullptr;
  YGNodeRef root_ = nullptr;
  std::unordered_map<const widget*, YGNodeRef> nodes_;
  std::vector<std::unique_ptr<node_payload>> payloads_;
};

[[nodiscard]] rect node_rect(YGNodeRef node, float parent_x, float parent_y) noexcept {
  return rect{ parent_x + YGNodeLayoutGetLeft(node),
               parent_y + YGNodeLayoutGetTop(node),
               finite_non_negative(YGNodeLayoutGetWidth(node)),
               finite_non_negative(YGNodeLayoutGetHeight(node)) };
}

void fill_bounds(widget& owner, YGNodeRef node, float parent_x, float parent_y, const tree_builder& builder) {
  layout_access::set_bounds(owner, node_rect(node, parent_x, parent_y));
  const rect bounds = owner.bounds();
  layout_access::visit_children(owner, [&](widget& child) {
    YGNodeRef child_node = builder.node(child);
    if (child_node != nullptr) {
      fill_bounds(child, child_node, bounds.x, bounds.y, builder);
    }
  });
}

void draw_tree(widget& owner, draw_context& context) {
  const std::string_view owner_id = layout_access::id(owner);
  if (!owner_id.empty()) {
    ImGui::PushID(owner_id.data(), owner_id.data() + owner_id.size());
  }

  layout_access::draw(owner, context);

  const bool clipped = layout_access::clips_children(owner);
  if (clipped) {
    clip_rect(context.draw_list, owner.bounds());
  }

  int index = 0;
  layout_access::visit_children(owner, [&](widget& child) {
    ImGui::PushID(index);
    draw_tree(child, context);
    ImGui::PopID();
    ++index;
  });

  if (clipped) {
    unclip_rect(context.draw_list);
  }

  if (!owner_id.empty()) {
    ImGui::PopID();
  }
}

} // namespace

void render(widget& root, ImVec2 origin, ImVec2 available, const theme& appearance) {
  if (!(available.x > 0.0f) || !(available.y > 0.0f)) {
    return;
  }

  tree_builder builder{ available, appearance };
  YGNodeRef root_node = builder.build(root);
  YGNodeCalculateLayout(root_node, finite_non_negative(available.x), finite_non_negative(available.y), YGDirectionLTR);
  fill_bounds(root, root_node, origin.x, origin.y, builder);

  draw_context context{ ImGui::GetWindowDrawList(), appearance };
  ImGui::PushID("cyberverse.ui.widgets");
  draw_tree(root, context);
  ImGui::PopID();

  // Reserve the root rectangle as a real ImGui item. Widgets draw with
  // absolute coordinates and move the cursor; without an item ImGui treats the
  // move as an attempt to extend the parent window's boundaries.
  ImGui::SetCursorScreenPos(origin);
  ImGui::Dummy(ImVec2{ std::max(1.0f, root.bounds().width), std::max(1.0f, root.bounds().height) });
}

measured_size measure(widget& root, const constraints& limits, const theme& appearance) {
  const float width = finite_non_negative(limits.available_width);
  const float height = finite_non_negative(limits.available_height);

  tree_builder builder{ ImVec2{ width, height }, appearance };
  YGNodeRef root_node = builder.build(root);
  YGNodeCalculateLayout(root_node, width, height, YGDirectionLTR);

  return measured_size{ finite_non_negative(YGNodeLayoutGetWidth(root_node)),
                        finite_non_negative(YGNodeLayoutGetHeight(root_node)) };
}

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
