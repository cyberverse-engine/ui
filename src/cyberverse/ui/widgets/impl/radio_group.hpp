/**
 * @file radio_group.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Radio group implementation
 * @version 0.1
 * @date 2026-10-31
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <utility>

#include <imgui.h>

#include "cyberverse/ui/widgets/component.hpp"
#include "cyberverse/ui/widgets/context.hpp"
#include "cyberverse/ui/widgets/detail/font.hpp"
#include "cyberverse/ui/widgets/detail/stack_layout.hpp"
#include "cyberverse/ui/widgets/event.hpp"
#include "cyberverse/ui/widgets/radio_group_types.hpp"
#include "cyberverse/ui/widgets/state.hpp"
#include "cyberverse/ui/widgets/theme.hpp"
#include "cyberverse/ui/widgets/types.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {
namespace impl {

/**
 * @brief Mutually exclusive group of radio/button options
 *
 */
template <std::size_t N>
struct radio_group {
  radio_group_options options;
  int* selected = nullptr;
  std::array<std::string, N> labels;

  [[nodiscard]] layout_size measure(const state& s, const theme& th, const constraints& /*lim*/) const noexcept {
    const detail::font_scope font(detail::resolve_font_size(s, th));
    std::array<layout_size, N> sizes{};
    std::array<bool, N> active{};
    active.fill(true);

    for (std::size_t index = 0; index < N; ++index) {
      sizes[index] = measure_option(labels[index], th, options.type);
    }

    layout_size result = detail::measure_stack(options.layout, sizes, active);
    if (s.has(property_type::width)) {
      result.width = std::max(0.0f, s.value(property_type::width));
    }
    if (s.has(property_type::height)) {
      result.height = std::max(0.0f, s.value(property_type::height));
    }
    return result;
  }

  void draw(state& s, widget_context& ctx) const {
    const rect& bounds = s.bounds();
    if (selected == nullptr || bounds.width <= 0.0f || bounds.height <= 0.0f) {
      return;
    }

    const detail::font_scope font(detail::resolve_font_size(s, ctx.appearance));
    std::array<layout_size, N> sizes{};
    std::array<bool, N> active{};
    active.fill(true);
    for (std::size_t index = 0; index < N; ++index) {
      sizes[index] = measure_option(labels[index], ctx.appearance, options.type);
    }

    const std::array<rect, N> placements = detail::place_stack(options.layout, sizes, active, bounds);
    ImGui::PushID(s.id().data(), s.id().data() + s.id().size());

    for (std::size_t index = 0; index < N; ++index) {
      const rect& item_bounds = placements[index];
      if (item_bounds.width <= 0.0f || item_bounds.height <= 0.0f) {
        continue;
      }

      ImGui::PushID(static_cast<int>(index));
      bool clicked = false;
      if (options.type == radio_group_type::radio) {
        clicked = draw_radio(item_bounds, labels[index], ctx, *selected == static_cast<int>(index));
      } else {
        clicked = draw_button(item_bounds, labels[index], ctx, *selected == static_cast<int>(index));
      }
      ImGui::PopID();

      if (clicked && *selected != static_cast<int>(index)) {
        *selected = static_cast<int>(index);
        s.emit(event_type::value_changed);
      }
    }

    ImGui::PopID();
  }

  template <typename F>
  void visit_states(F&& /*visitor*/) const noexcept {}

private:
  [[nodiscard]] static layout_size
  measure_option(std::string_view label, const theme& th, radio_group_type type) noexcept {
    if (type == radio_group_type::radio) {
      const choice_style& style = th.radio;
      const float frame_height = ImGui::GetFrameHeight();
      const ImVec2 label_size = ImGui::CalcTextSize(label.data(), label.data() + label.size());
      const float spacing = ImGui::GetStyle().ItemInnerSpacing.x;
      const float width = frame_height + spacing + label_size.x + 2.0f * style.padding_x;
      const float height = std::max(frame_height, label_size.y) + 2.0f * style.padding_y;
      return layout_size{ std::max(0.0f, width), std::max(0.0f, height) };
    }

    const button_style& style = th.button;
    const ImVec2 text_size = ImGui::CalcTextSize(label.data(), label.data() + label.size());
    const float width = text_size.x + 2.0f * style.padding_x;
    const float height = std::max(text_size.y + 2.0f * style.padding_y, ImGui::GetFrameHeight());
    return layout_size{ std::max(0.0f, width), std::max(0.0f, height) };
  }

  [[nodiscard]] static bool
  draw_radio(const rect& bounds, const std::string& label, const widget_context& ctx, bool selected_value) {
    const choice_style& style = ctx.appearance.radio;
    ImGui::PushStyleColor(ImGuiCol_FrameBg, detail::to_u32(style.frame));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, detail::to_u32(style.frame_hovered));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, detail::to_u32(style.frame_active));
    ImGui::PushStyleColor(ImGuiCol_Border, detail::to_u32(style.border));
    ImGui::PushStyleColor(ImGuiCol_Text, detail::to_u32(style.text));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, detail::to_u32(style.check));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.border_width);
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(ctx.opacity, 0.0f, 1.0f));

    ImGui::SetCursorScreenPos(ImVec2{ bounds.x + style.padding_x, bounds.y + style.padding_y });
    const bool clicked = ImGui::RadioButton(label.c_str(), selected_value);

    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(6);
    return clicked;
  }

  [[nodiscard]] static bool
  draw_button(const rect& bounds, const std::string& label, const widget_context& ctx, bool selected_value) {
    const button_style& style = ctx.appearance.button;
    const radio_group_style& selected_style = ctx.appearance.radio_group;
    const color& background = selected_value ? selected_style.selected_background : style.background;
    const color& border = selected_value ? selected_style.selected_border : style.background;
    const color& text = selected_value ? selected_style.selected_text : style.text;

    ImGui::PushStyleColor(ImGuiCol_Button, detail::to_u32(background));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, detail::to_u32(background));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, detail::to_u32(background));
    ImGui::PushStyleColor(ImGuiCol_Border, detail::to_u32(border));
    ImGui::PushStyleColor(ImGuiCol_Text, detail::to_u32(text));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, style.rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, style.border_width);
    ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ style.padding_x, style.padding_y });
    ImGui::PushStyleVar(ImGuiStyleVar_Alpha, std::clamp(ctx.opacity, 0.0f, 1.0f));

    ImGui::SetCursorScreenPos(ImVec2{ bounds.x, bounds.y });
    const bool clicked =
        ImGui::Button(label.c_str(), ImVec2{ std::max(1.0f, bounds.width), std::max(1.0f, bounds.height) });

    ImGui::PopStyleVar(4);
    ImGui::PopStyleColor(5);
    return clicked;
  }
};

} // namespace impl
} // namespace widgets
} // namespace ui
} // namespace cyberverse
