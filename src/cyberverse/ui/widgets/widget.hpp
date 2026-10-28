/**
 * @file widget.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Base widget, sizing model, and per-frame widget tree
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <cstdint>
#include <functional>
#include <limits>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "cyberverse/ui/panel.hpp"
#include "cyberverse/ui/widgets/theme.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

namespace detail {
class layout_engine;
struct constraints;
struct draw_context;
struct layout_access;
struct measured_size;
struct measure_context;
} // namespace detail

/**
 * @brief Main-axis direction of a flexbox container
 *
 */
enum class flex_direction { row, row_reverse, column, column_reverse };

/**
 * @brief Main-axis distribution
 *
 */
enum class justify_content { start, center, end, space_between };

/**
 * @brief Cross-axis alignment
 *
 */
enum class align_items { start, center, end, stretch };

/**
 * @brief Width/height sizing policy
 *
 */
enum class size_mode {
  fit_content, ///< Measured from content
  fixed,       ///< Exact logical points
  stretch,     ///< Fill the parent on the main axis / stretch on the cross axis
};

/**
 * @brief Width or height sizing policy with its fixed value
 *
 */
struct size_spec {
  size_mode mode = size_mode::fit_content;
  float value = 0.0f;
};

/**
 * @brief Four-sided inset in logical points
 *
 */
struct insets {
  float left = 0.0f;
  float top = 0.0f;
  float right = 0.0f;
  float bottom = 0.0f;
};

/**
 * @brief Explicit content size used by measure callbacks
 *
 */
struct content_size {
  float width = 0.0f;
  float height = 0.0f;
};

/**
 * @brief Base of every standard widget
 *
 * Widgets form a per-frame tree built inside `widgets::render(root, theme)`.
 * The tree is consumed immediately: measurement, layout, drawing, and
 * interaction all happen in that call. Persistent state (text buffers, checked
 * values, selected radio values) is supplied by the application as external
 * references.
 *
 */
class widget {
public:
  virtual ~widget() = default;

  widget(const widget&) = delete;
  widget(widget&&) = default;
  widget& operator=(const widget&) = delete;
  widget& operator=(widget&&) = default;

  /**
   * @brief Rectangle assigned by the last layout pass, in screen coordinates
   *
   */
  [[nodiscard]] const rect& bounds() const noexcept { return bounds_; }

  /**
   * @brief Stable identifier used for Dear ImGui IDs
   *
   */
  [[nodiscard]] std::string_view id() const noexcept { return id_; }

protected:
  using child_visitor = std::function<void(widget&)>;

  friend class detail::layout_engine;
  friend struct detail::layout_access;

  widget() = default;

  size_spec width_{};
  size_spec height_{};
  float min_width_ = 0.0f;
  float max_width_ = std::numeric_limits<float>::infinity();
  float min_height_ = 0.0f;
  float max_height_ = std::numeric_limits<float>::infinity();
  insets margin_{};
  std::optional<float> grow_{};
  std::optional<float> shrink_{};
  std::optional<widget_style> style_override_{};
  std::string id_;
  rect bounds_{};

private:
  virtual detail::measured_size measure(detail::measure_context& context, const detail::constraints& limits) = 0;
  virtual void draw(detail::draw_context& context) = 0;
  virtual void visit_children(const child_visitor& visitor) const = 0;

  [[nodiscard]] virtual bool is_measure_leaf() const noexcept { return true; }
  [[nodiscard]] virtual bool clips_children() const noexcept { return false; }
  [[nodiscard]] virtual flex_direction flow_direction() const noexcept { return flex_direction::column; }
  [[nodiscard]] virtual insets flow_padding() const noexcept { return {}; }
  [[nodiscard]] virtual float flow_gap() const noexcept { return 0.0f; }
  [[nodiscard]] virtual justify_content flow_justify() const noexcept { return justify_content::start; }
  [[nodiscard]] virtual align_items flow_align() const noexcept { return align_items::start; }
};

/**
 * @brief Fluent sizing/style API shared by every concrete widget
 *
 * Setters return the concrete component type so construction can be chained,
 * for example `button{"OK", cb}.width_stretch()`.
 *
 */
template <typename Derived>
class basic_widget : public widget {
public:
  Derived& width(float value) {
    width_ = size_spec{ size_mode::fixed, value };
    return self();
  }

  Derived& height(float value) {
    height_ = size_spec{ size_mode::fixed, value };
    return self();
  }

  Derived& size(float width_value, float height_value) {
    width_ = size_spec{ size_mode::fixed, width_value };
    height_ = size_spec{ size_mode::fixed, height_value };
    return self();
  }

  Derived& width_fit_content() {
    width_.mode = size_mode::fit_content;
    width_.value = 0.0f;
    return self();
  }

  Derived& height_fit_content() {
    height_.mode = size_mode::fit_content;
    height_.value = 0.0f;
    return self();
  }

  Derived& width_stretch() {
    width_.mode = size_mode::stretch;
    width_.value = 0.0f;
    return self();
  }

  Derived& height_stretch() {
    height_.mode = size_mode::stretch;
    height_.value = 0.0f;
    return self();
  }

  Derived& min_width(float value) {
    min_width_ = value;
    return self();
  }

  Derived& max_width(float value) {
    max_width_ = value;
    return self();
  }

  Derived& min_height(float value) {
    min_height_ = value;
    return self();
  }

  Derived& max_height(float value) {
    max_height_ = value;
    return self();
  }

  Derived& margin(insets value) {
    margin_ = value;
    return self();
  }

  Derived& grow(float value) {
    grow_ = value;
    return self();
  }

  Derived& shrink(float value) {
    shrink_ = value;
    return self();
  }

  Derived& id(std::string_view value) {
    id_.assign(value.data(), value.size());
    return self();
  }

  Derived& style(widget_style value) {
    style_override_ = std::move(value);
    return self();
  }

private:
  Derived& self() noexcept { return static_cast<Derived&>(*this); }
};

/**
 * @brief Run one widget tree and render it into the current Dear ImGui window
 *
 * The tree is laid out inside the available content region at the current
 * cursor, drawn, and then discarded. After the call the ImGui cursor is moved
 * below the root widget. Must be called between ImGui::NewFrame() and
 * ImGui::Render(), normally from ui::panel::draw().
 *
 * @param root Root widget
 * @param appearance Optional theme overrides; missing values come from ImGuiStyle
 */
void render(widget& root, const theme& appearance = {});

} // namespace widgets
} // namespace ui
} // namespace cyberverse
