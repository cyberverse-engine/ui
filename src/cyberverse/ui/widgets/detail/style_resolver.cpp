/**
 * @file style_resolver.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Theme/style resolution against the active ImGuiStyle
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/detail/style_resolver.hpp"

#include <algorithm>
#include <cmath>

namespace cyberverse {
namespace ui {
namespace widgets {
namespace detail {

namespace {

[[nodiscard]] ImU32 to_u32(const ImVec4& value) noexcept { return ImGui::ColorConvertFloat4ToU32(value); }

[[nodiscard]] float positive(float value, float fallback) noexcept {
  return (std::isfinite(value) && value >= 0.0f) ? value : fallback;
}

const widget_style& theme_style(const theme& appearance, style_kind kind) noexcept {
  switch (kind) {
  case style_kind::panel:
    return appearance.panel;
  case style_kind::divider:
    return appearance.divider;
  case style_kind::button:
    return appearance.button;
  case style_kind::checkbox:
    return appearance.checkbox;
  case style_kind::radio:
    return appearance.radio;
  case style_kind::text_input:
    return appearance.text_input;
  case style_kind::scroll_view:
    return appearance.scroll_view;
  case style_kind::collapse_header:
    return appearance.collapse_header;
  case style_kind::collapse_body:
    return appearance.collapse_body;
  case style_kind::text:
  default:
    return appearance.text;
  }
}

void merge(widget_style& target, const widget_style& source) noexcept {
  if (source.background) {
    target.background = source.background;
  }
  if (source.background_hovered) {
    target.background_hovered = source.background_hovered;
  }
  if (source.background_active) {
    target.background_active = source.background_active;
  }
  if (source.text) {
    target.text = source.text;
  }
  if (source.text_disabled) {
    target.text_disabled = source.text_disabled;
  }
  if (source.border) {
    target.border = source.border;
  }
  if (source.focus_ring) {
    target.focus_ring = source.focus_ring;
  }
  if (source.padding_x) {
    target.padding_x = source.padding_x;
  }
  if (source.padding_y) {
    target.padding_y = source.padding_y;
  }
}

struct defaults {
  ImVec4 background;
  ImVec4 background_hovered;
  ImVec4 background_active;
  ImVec4 text;
  ImVec4 text_disabled;
  ImVec4 border;
  ImVec4 focus;
  float border_width = 0.0f;
  float radius = 0.0f;
  float padding_x = 0.0f;
  float padding_y = 0.0f;
  bool has_background = true;
};

[[nodiscard]] defaults defaults_for(style_kind kind, const ImGuiStyle& style) noexcept {
  const auto color = [&style](ImGuiCol index) {
    return style.Colors[index];
  };

  switch (kind) {
  case style_kind::panel:
    return { color(ImGuiCol_WindowBg),
             color(ImGuiCol_WindowBg),
             color(ImGuiCol_WindowBg),
             color(ImGuiCol_Text),
             color(ImGuiCol_TextDisabled),
             color(ImGuiCol_Border),
             color(ImGuiCol_NavCursor),
             style.WindowBorderSize,
             style.WindowRounding,
             0.0f,
             0.0f,
             true };
  case style_kind::button:
    return { color(ImGuiCol_Button),    color(ImGuiCol_ButtonHovered), color(ImGuiCol_ButtonActive),
             color(ImGuiCol_Text),      color(ImGuiCol_TextDisabled),  color(ImGuiCol_Border),
             color(ImGuiCol_NavCursor), style.FrameBorderSize,         style.FrameRounding,
             style.FramePadding.x,      style.FramePadding.y,          true };
  case style_kind::checkbox:
  case style_kind::radio:
    return { color(ImGuiCol_FrameBg),
             color(ImGuiCol_FrameBgHovered),
             color(ImGuiCol_FrameBgActive),
             color(ImGuiCol_Text),
             color(ImGuiCol_TextDisabled),
             color(ImGuiCol_Border),
             color(ImGuiCol_NavCursor),
             style.FrameBorderSize,
             style.FrameRounding,
             0.0f,
             0.0f,
             true };
  case style_kind::text_input:
    return { color(ImGuiCol_FrameBg),   color(ImGuiCol_FrameBgHovered), color(ImGuiCol_FrameBgActive),
             color(ImGuiCol_Text),      color(ImGuiCol_TextDisabled),   color(ImGuiCol_Border),
             color(ImGuiCol_NavCursor), style.FrameBorderSize,          style.FrameRounding,
             style.FramePadding.x,      style.FramePadding.y,           true };
  case style_kind::scroll_view:
    return { color(ImGuiCol_ChildBg),
             color(ImGuiCol_ChildBg),
             color(ImGuiCol_ChildBg),
             color(ImGuiCol_Text),
             color(ImGuiCol_TextDisabled),
             color(ImGuiCol_Border),
             color(ImGuiCol_NavCursor),
             style.ChildBorderSize,
             style.ChildRounding,
             0.0f,
             0.0f,
             true };
  case style_kind::collapse_header:
    return { color(ImGuiCol_Header),    color(ImGuiCol_HeaderHovered), color(ImGuiCol_HeaderActive),
             color(ImGuiCol_Text),      color(ImGuiCol_TextDisabled),  color(ImGuiCol_Border),
             color(ImGuiCol_NavCursor), style.FrameBorderSize,         style.FrameRounding,
             style.FramePadding.x,      style.FramePadding.y,          true };
  case style_kind::collapse_body:
    return { color(ImGuiCol_ChildBg),
             color(ImGuiCol_ChildBg),
             color(ImGuiCol_ChildBg),
             color(ImGuiCol_Text),
             color(ImGuiCol_TextDisabled),
             color(ImGuiCol_Border),
             color(ImGuiCol_NavCursor),
             0.0f,
             style.ChildRounding,
             0.0f,
             0.0f,
             true };
  case style_kind::text:
  default:
    return { color(ImGuiCol_Text),
             color(ImGuiCol_Text),
             color(ImGuiCol_Text),
             color(ImGuiCol_Text),
             color(ImGuiCol_TextDisabled),
             color(ImGuiCol_Border),
             color(ImGuiCol_NavCursor),
             0.0f,
             0.0f,
             0.0f,
             0.0f,
             false };
  }
}

} // namespace

resolved_style resolve_style(style_kind kind, const theme& appearance, const std::optional<widget_style>& override) {
  const ImGuiStyle& style = ImGui::GetStyle();
  defaults fallback = defaults_for(kind, style);

  widget_style merged = theme_style(appearance, kind);
  if (override) {
    merge(merged, *override);
  }

  const color background = merged.background.value_or(
      color{ fallback.background.x, fallback.background.y, fallback.background.z, fallback.background.w });
  color background_hovered_value =
      merged.background_hovered.value_or(merged.background ? *merged.background
                                                           : color{ fallback.background_hovered.x,
                                                                    fallback.background_hovered.y,
                                                                    fallback.background_hovered.z,
                                                                    fallback.background_hovered.w });
  color background_active_value =
      merged.background_active.value_or(merged.background ? *merged.background
                                                          : color{ fallback.background_active.x,
                                                                   fallback.background_active.y,
                                                                   fallback.background_active.z,
                                                                   fallback.background_active.w });
  const color text_color =
      merged.text.value_or(color{ fallback.text.x, fallback.text.y, fallback.text.z, fallback.text.w });
  const color text_disabled_color = merged.text_disabled.value_or(
      color{ fallback.text_disabled.x, fallback.text_disabled.y, fallback.text_disabled.z, fallback.text_disabled.w });
  const color border_color = merged.border
                                 ? merged.border->color
                                 : color{ fallback.border.x, fallback.border.y, fallback.border.z, fallback.border.w };
  const color focus_color =
      merged.focus_ring.value_or(color{ fallback.focus.x, fallback.focus.y, fallback.focus.z, fallback.focus.w });

  resolved_style result;
  result.background = to_u32(ImVec4{ background.r, background.g, background.b, background.a });
  result.background_hovered = to_u32(ImVec4{
      background_hovered_value.r, background_hovered_value.g, background_hovered_value.b, background_hovered_value.a });
  result.background_active = to_u32(ImVec4{
      background_active_value.r, background_active_value.g, background_active_value.b, background_active_value.a });
  result.text = to_u32(ImVec4{ text_color.r, text_color.g, text_color.b, text_color.a });
  result.text_disabled =
      to_u32(ImVec4{ text_disabled_color.r, text_disabled_color.g, text_disabled_color.b, text_disabled_color.a });
  result.border = to_u32(ImVec4{ border_color.r, border_color.g, border_color.b, border_color.a });
  result.focus_ring = to_u32(ImVec4{ focus_color.r, focus_color.g, focus_color.b, focus_color.a });
  result.border_width = positive(merged.border ? merged.border->width : fallback.border_width, 0.0f);
  result.radius = positive(merged.border ? merged.border->radius : fallback.radius, 0.0f);
  result.padding_x = positive(merged.padding_x.value_or(fallback.padding_x), 0.0f);
  result.padding_y = positive(merged.padding_y.value_or(fallback.padding_y), 0.0f);
  result.has_background = fallback.has_background || merged.background.has_value();
  return result;
}

const theme& appearance_or_default(const theme* appearance) noexcept {
  static const theme empty;
  return appearance != nullptr ? *appearance : empty;
}

} // namespace detail
} // namespace widgets
} // namespace ui
} // namespace cyberverse
