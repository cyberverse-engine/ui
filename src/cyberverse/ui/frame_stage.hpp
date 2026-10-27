/**
 * @file frame_stage.hpp
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

#include "cyberverse/ui/panel.hpp"

namespace cyberverse {
namespace ui {

namespace detail {
class frame_stage_access;
} // namespace detail

/**
 * @brief Description used to create a UI layer
 *
 */
struct frame_stage_desc {
  std::string name;    ///< Unique layer name; no "###" or "/"
  int order = 0;       ///< Draw order; lower draws first
  bool visible = true; ///< Hidden layers are skipped entirely
};

/**
 * @brief One UI layer drawn inside the shared Dear ImGui frame
 *
 * A layer only owns a name, order, visibility, and an ordered list of panels.
 * It knows nothing about HUD, menus, diagnostics, or editor concepts. All
 * panels of a layer are managed Dear ImGui windows that fill the layer's
 * logical viewport; panel content positions itself with native Dear ImGui APIs.
 *
 * Layers are drawn from the lowest order to the highest; ties keep insertion
 * order. A hidden layer is not laid out and not drawn.
 *
 */
class frame_stage {
public:
  /**
   * @param desc Layer description
   * @throws std::invalid_argument If the name is empty or contains "###" or "/"
   */
  explicit frame_stage(frame_stage_desc desc);
  ~frame_stage();

  frame_stage(const frame_stage&) = delete;
  frame_stage(frame_stage&&) = delete;
  frame_stage& operator=(const frame_stage&) = delete;
  frame_stage& operator=(frame_stage&&) = delete;

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
   * @brief Add a panel to the layer
   *
   * Ownership transfers to the layer. The returned reference is valid until
   * the panel is removed or the layer is cleared.
   *
   * @throws std::invalid_argument If the panel is null, its id is invalid, or
   *         the id is already used in this layer
   * @throws std::logic_error If called during an active UI frame
   */
  panel& add_panel(std::unique_ptr<panel> content);

  /**
   * @brief Remove a panel owned by this layer
   *
   * @return true when the panel was owned and removed
   */
  bool remove_panel(panel& content) noexcept;

  /**
   * @brief Remove every panel
   *
   * @throws std::logic_error If called during an active UI frame
   */
  void clear_panels();

  /**
   * @brief Whether at least one panel reports visible()
   *
   */
  [[nodiscard]] bool has_visible_panels() const noexcept;

private:
  friend class detail::frame_stage_access;

  std::string name_;
  int order_ = 0;
  bool visible_ = true;
  std::vector<std::unique_ptr<panel>> panels_;
};

} // namespace ui
} // namespace cyberverse
