/**
 * @file state.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Persistent per-component state implementation
 * @version 0.1
 * @date 2026-10-30
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets2/state.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <utility>

#include "cyberverse/animation/lerp.hpp"
#include "cyberverse/ui/widgets2/context.hpp"
#include "cyberverse/ui/widgets2/detail/state_access.hpp"

namespace cyberverse {
namespace ui {
namespace widgets2 {

namespace {

[[nodiscard]] bool finite(float value) noexcept { return std::isfinite(value); }

} // namespace

state::state(std::string id) : id_(std::move(id)) {
  if (id_.empty()) {
    throw std::invalid_argument("cyberverse::ui::widgets2::state: id must not be empty");
  }

  property_entry& opacity = entry(property_type::opacity);
  opacity.current = 1.0f;
  opacity.from = 1.0f;
  opacity.target = 1.0f;
  opacity.has_value = true;
}

state::property_entry& state::entry(property_type property) noexcept {
  return properties_[static_cast<std::size_t>(property)];
}

const state::property_entry& state::entry(property_type property) const noexcept {
  return properties_[static_cast<std::size_t>(property)];
}

bool state::has(property_type property) const noexcept { return entry(property).has_value; }

float state::value(property_type property) const noexcept {
  const property_entry& data = entry(property);
  return data.has_value ? data.current : 0.0f;
}

bool state::has_zero_dimension() const noexcept {
  const bool zero_width = has(property_type::width) && value(property_type::width) <= 0.0f;
  const bool zero_height = has(property_type::height) && value(property_type::height) <= 0.0f;
  return zero_width || zero_height;
}

void state::set(property_type property, float value) noexcept {
  if (!finite(value)) {
    return;
  }
  property_entry& data = entry(property);
  data.current = value;
  data.from = value;
  data.target = value;
  data.elapsed = 0.0f;
  data.duration = 0.0f;
  data.delay = 0.0f;
  data.active = false;
  data.has_value = true;
}

void state::animate(property_type property, float target, animation_params params) noexcept {
  if (!finite(target)) {
    return;
  }

  property_entry& data = entry(property);
  if (!data.has_value) {
    set(property, target);
    return;
  }

  if (!finite(params.duration) || params.duration < 0.0f) {
    params.duration = 0.0f;
  }
  if (!finite(params.delay) || params.delay < 0.0f) {
    params.delay = 0.0f;
  }

  data.from = data.current;
  data.target = target;
  data.elapsed = 0.0f;
  data.duration = params.duration;
  data.delay = params.delay;
  data.easing = params.easing;
  data.active = params.duration > 0.0f || params.delay > 0.0f;

  if (!data.active) {
    data.current = target;
  }
}

void state::on_event(std::function<void(state&, event_type)> handler) { event_handler_ = std::move(handler); }

void state::emit(event_type event) {
  if (event_handler_) {
    event_handler_(*this, event);
  }
}

void state::tick(update_context& ctx) noexcept {
  if (ctx.animations.paused()) {
    return;
  }

  float dt = ctx.dt * ctx.animations.speed();
  if (!(dt > 0.0f)) {
    dt = 0.0f;
  }

  for (property_entry& data : properties_) {
    if (!data.active) {
      continue;
    }

    if (data.delay > 0.0f) {
      if (dt < data.delay) {
        data.delay -= dt;
        continue;
      }
      dt -= data.delay;
      data.delay = 0.0f;
    }

    if (data.duration <= 0.0f) {
      data.current = data.target;
      data.active = false;
      continue;
    }

    data.elapsed += dt;
    float progress = data.elapsed / data.duration;
    if (!(progress > 0.0f)) {
      progress = 0.0f;
    }
    if (progress >= 1.0f) {
      data.current = data.target;
      data.active = false;
      continue;
    }

    data.current = animation::lerp_float(data.from, data.target, animation::apply_easing(data.easing, progress));
  }
}

void detail::state_access::set_bounds(state& owner, const rect& value) noexcept { owner.bounds_ = value; }

} // namespace widgets2
} // namespace ui
} // namespace cyberverse
