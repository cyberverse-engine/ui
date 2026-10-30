/**
 * @file component.hpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Static component wrapper and concepts
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#pragma once

#include <algorithm>
#include <concepts>
#include <utility>

#include "cyberverse/ui/widgets/context.hpp"
#include "cyberverse/ui/widgets/state.hpp"
#include "cyberverse/ui/widgets/theme.hpp"
#include "cyberverse/ui/widgets/types.hpp"
#include "cyberverse/ui/widgets/visibility.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

/**
 * @brief Measure result for a component with a known-zero dimension
 *
 */
[[nodiscard]] inline layout_size zero_measure(const state& s) noexcept {
  const float width = s.has(property_type::width) ? std::max(0.0f, s.value(property_type::width)) : 0.0f;
  const float height = s.has(property_type::height) ? std::max(0.0f, s.value(property_type::height)) : 0.0f;
  return layout_size{ width, height };
}

/**
 * @brief Requirements on a component implementation type
 *
 */
template <typename Impl>
concept impl_component = requires(Impl& impl, state& s, const theme& th, const constraints& lim, widget_context& ctx) {
  { impl.measure(s, th, lim) } -> std::convertible_to<layout_size>;
  impl.draw(s, ctx);
  impl.visit_states([](state&) {
  });
};

/**
 * @brief Static wrapper binding a state to an implementation
 *
 * This is intentionally not a polymorphic base. It owns only a state reference
 * and the implementation value; both live on the caller's stack (unless the
 * caller chooses otherwise).
 *
 */
template <typename Impl>
class component {
public:
  state& s;

  component(state& owner, Impl impl) : s(owner), impl_(std::move(impl)) {
    static_assert(impl_component<Impl>, "widgets impl does not satisfy impl_component");
  }

  [[nodiscard]] visibility visibility_value() const noexcept { return s.get_visibility(); }

  [[nodiscard]] layout_size measure(const theme& th, const constraints& lim) const {
    if (s.get_visibility() == visibility::collapsed) {
      return layout_size{ 0.0f, 0.0f };
    }
    if (s.has_zero_dimension()) {
      return zero_measure(s);
    }
    return impl_.measure(s, th, lim);
  }

  void draw(widget_context& ctx) {
    const rect& bounds = s.bounds();
    if (s.get_visibility() != visibility::visible || bounds.width <= 0.0f || bounds.height <= 0.0f) {
      return;
    }

    const float previous = ctx.opacity;
    ctx.opacity *= std::clamp(s.value(property_type::opacity), 0.0f, 1.0f);
    impl_.draw(s, ctx);
    ctx.opacity = previous;
  }

  void tick(widget_context& ctx) {
    const float dt = static_cast<float>(ctx.window.delta_time);
    visit_states([&](state& value) {
      value.tick(dt);
    });
  }

  template <typename F>
  void visit_states(F&& f) {
    f(s);
    impl_.visit_states(f);
  }

private:
  Impl impl_;
};

/**
 * @brief Requirements on a complete component
 *
 */
template <typename T>
concept component_like = requires(T& c, const theme& th, const constraints& lim, widget_context& ctx) {
  { c.s } -> std::same_as<state&>;
  { c.visibility_value() } -> std::same_as<visibility>;
  { c.measure(th, lim) } -> std::same_as<layout_size>;
  c.draw(ctx);
  c.tick(ctx);
};

} // namespace widgets
} // namespace ui
} // namespace cyberverse
