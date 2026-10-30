/**
 * @file layer.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief One UI layer
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <memory>
#include <string>
#include <string_view>
#include <vector>

#include "cyberverse/ui/window.hpp"

namespace cyberverse {
namespace ui {

namespace detail {
class layer_access;
} // namespace detail

/**
 * @brief Description used to create a UI layer
 *
 */
struct layer_desc {
  std::string name;    ///< Unique layer name; no "###" or "/"
  int order = 0;       ///< Draw order; lower draws first
  bool visible = true; ///< Hidden layers are skipped entirely
};

/**
 * @brief One UI layer drawn inside the shared Dear ImGui frame
 *
 * A layer only owns a name, order, visibility, and an ordered list of windows.
 * It knows nothing about HUD, menus, diagnostics, or editor concepts. All
 * windows of a layer are managed Dear ImGui windows that fill the layer's
 * logical viewport; window content positions itself with native Dear ImGui APIs.
 *
 * Layers are drawn from the lowest order to the highest; ties keep insertion
 * order. A hidden layer is not laid out and not drawn.
 *
 */
class layer {
public:
  /**
   * @param desc Layer description
   * @throws std::invalid_argument If the name is empty or contains "###" or "/"
   */
  explicit layer(layer_desc desc);
  ~layer();

  layer(const layer&) = delete;
  layer(layer&&) = delete;
  layer& operator=(const layer&) = delete;
  layer& operator=(layer&&) = delete;

  [[nodiscard]] std::string_view name() const noexcept;

  [[nodiscard]] int order() const noexcept;

  /**
   * @brief Change draw order between frames
   *
   * @throws std::logic_error If called during an active UI frame
   */
  void set_order(int order);

  [[nodiscard]] bool visible() const noexcept;

  /**
   * @brief Show or hide the whole layer between frames
   *
   * @throws std::logic_error If called during an active UI frame
   */
  void set_visible(bool visible);

  /**
   * @brief Add a window to the layer
   *
   * Ownership transfers to the layer. The returned reference is valid until
   * the window is removed or the layer is cleared.
   *
   * @throws std::invalid_argument If the window is null, its id is invalid, or
   *         the id is already used in this layer
   * @throws std::logic_error If called during an active UI frame
   */
  window& add_window(std::unique_ptr<window> content);

  /**
   * @brief Remove a window owned by this layer
   *
   * @return true when the window was owned and removed
   */
  bool remove_window(window& content) noexcept;

  /**
   * @brief Remove every window
   *
   * @throws std::logic_error If called during an active UI frame
   */
  void clear_windows();

  /**
   * @brief Whether at least one window reports visible()
   *
   */
  [[nodiscard]] bool has_visible_windows() const noexcept;

private:
  friend class detail::layer_access;

  std::string name_;
  int order_ = 0;
  bool visible_ = true;
  std::vector<std::unique_ptr<window>> windows_;
};

} // namespace ui
} // namespace cyberverse
