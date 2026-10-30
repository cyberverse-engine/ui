/**
 * @file state.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Persistent per-component state
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <array>
#include <functional>
#include <string>
#include <string_view>

#include "cyberverse/animation/easing.hpp"
#include "cyberverse/ui/widgets/event.hpp"
#include "cyberverse/ui/widgets/property.hpp"
#include "cyberverse/ui/widgets/rect.hpp"
#include "cyberverse/ui/widgets/visibility.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

struct update_context;
namespace detail {
struct state_access;
}

/**
 * @brief Persistent state for one component
 *
 * State is caller-owned, non-copyable/non-movable, and stores all mutable
 * component data: layout bounds, property values and their animation metadata,
 * visibility, and the event handler. It does not own child components or the
 * global animator.
 *
 */
class state {
public:
  explicit state(std::string id);

  state(const state&) = delete;
  state(state&&) = delete;
  state& operator=(const state&) = delete;
  state& operator=(state&&) = delete;

  [[nodiscard]] std::string_view id() const noexcept { return id_; }
  [[nodiscard]] const rect& bounds() const noexcept { return bounds_; }

  [[nodiscard]] bool has(property_type property) const noexcept;
  [[nodiscard]] float value(property_type property) const noexcept;

  /**
   * @brief True when an explicit width or height is zero or negative
   *
   * Containers use this to skip child measurement and layout entirely.
   */
  [[nodiscard]] bool has_zero_dimension() const noexcept;

  /**
   * @brief Set a property immediately, without animation
   */
  void set(property_type property, float value) noexcept;

  /**
   * @brief Animate a property toward a target
   *
   * When no value has been set yet, the target is applied immediately. Use
   * set() first to establish a start value for an entrance animation.
   */
  void animate(property_type property, float target, animation_params params = {}) noexcept;

  [[nodiscard]] visibility get_visibility() const noexcept { return visibility_; }
  void set_visibility(visibility value) noexcept { visibility_ = value; }

  /**
   * @brief Set the persistent event handler
   *
   * This is the one intentional std::function allocation point. The handler is
   * not copied during event emission.
   */
  void on_event(std::function<void(state&, event_type)> handler);

  void emit(event_type event);

  /**
   * @brief Advance all active properties by one frame
   */
  void tick(update_context& ctx) noexcept;

private:
  friend struct detail::state_access;

  struct property_entry {
    float current = 0.0f;
    float from = 0.0f;
    float target = 0.0f;
    float elapsed = 0.0f;
    float duration = 0.0f;
    float delay = 0.0f;
    animation::easing_type easing = animation::easing_type::ease_out_cubic;
    bool active = false;
    bool has_value = false;
  };

  [[nodiscard]] property_entry& entry(property_type property) noexcept;
  [[nodiscard]] const property_entry& entry(property_type property) const noexcept;

  std::string id_;
  rect bounds_{};
  visibility visibility_ = visibility::visible;
  std::array<property_entry, property_count> properties_{};
  std::function<void(state&, event_type)> event_handler_;
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
