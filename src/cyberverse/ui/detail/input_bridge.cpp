/**
 * @file input_bridge.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief platform raw input to Dear ImGui event bridge
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/detail/input_bridge.hpp"

#include <string_view>

#include "cyberverse/ui/detail/key_map.hpp"

namespace cyberverse {
namespace ui {
namespace detail {

namespace {

// Decode one UTF-8 code point. Returns 0 and advances by one byte on malformed
// input so a broken sequence cannot stall the stream.
char32_t decode_utf8(std::string_view text, std::size_t& index) noexcept {
  const auto byte = [&text](std::size_t at) {
    return static_cast<unsigned char>(text[at]);
  };

  const unsigned char first = byte(index);
  if (first < 0x80) {
    ++index;
    return first;
  }

  int length = 0;
  char32_t codepoint = 0;
  if ((first & 0xE0) == 0xC0) {
    length = 2;
    codepoint = first & 0x1Fu;
  } else if ((first & 0xF0) == 0xE0) {
    length = 3;
    codepoint = first & 0x0Fu;
  } else if ((first & 0xF8) == 0xF0) {
    length = 4;
    codepoint = first & 0x07u;
  } else {
    ++index;
    return 0;
  }

  if (index + static_cast<std::size_t>(length) > text.size()) {
    ++index;
    return 0;
  }

  for (int offset = 1; offset < length; ++offset) {
    const unsigned char continuation = byte(index + static_cast<std::size_t>(offset));
    if ((continuation & 0xC0) != 0x80) {
      ++index;
      return 0;
    }
    codepoint = (codepoint << 6) | (continuation & 0x3Fu);
  }

  index += static_cast<std::size_t>(length);

  if ((length == 2 && codepoint < 0x80) || (length == 3 && codepoint < 0x800) || (length == 4 && codepoint < 0x10000) ||
      codepoint > 0x10FFFF || (codepoint >= 0xD800 && codepoint <= 0xDFFF)) {
    return 0;
  }

  return codepoint;
}

} // namespace

void input_bridge::update(ImGuiIO& io, const platform::raw_input_state& input) {
  const glm::vec2 mouse = input.mouse_position();
  io.AddMousePosEvent(mouse.x, mouse.y);

  for (int button = 0; button < mouse_button_count; ++button) {
    const bool down = input.is_mouse_button_down(button);
    if (down != mouse_down_.test(static_cast<std::size_t>(button))) {
      io.AddMouseButtonEvent(button, down);
      mouse_down_.set(static_cast<std::size_t>(button), down);
    }
  }

  const glm::vec2 scroll = input.scroll_delta();
  if (scroll.x != 0.0f || scroll.y != 0.0f) {
    io.AddMouseWheelEvent(scroll.x, scroll.y);
  }

  const key_binding* bindings = key_bindings();
  const std::size_t binding_count = key_binding_count();
  for (std::size_t index = 0; index < binding_count; ++index) {
    const key_binding& binding = bindings[index];
    const auto slot = static_cast<std::size_t>(binding.key);
    const bool down = input.is_key_down(binding.key);
    if (down != keys_down_.test(slot)) {
      io.AddKeyEvent(binding.imgui, down);
      keys_down_.set(slot, down);
    }
  }

  const auto update_modifier = [&input, &io](platform::mod_keys bit, ImGuiKey key, bool& state) {
    const bool down = input.is_mod_down(bit);
    if (down != state) {
      io.AddKeyEvent(key, down);
      state = down;
    }
  };
  update_modifier(platform::mod_keys::control, ImGuiMod_Ctrl, control_down_);
  update_modifier(platform::mod_keys::shift, ImGuiMod_Shift, shift_down_);
  update_modifier(platform::mod_keys::alt, ImGuiMod_Alt, alt_down_);
  update_modifier(platform::mod_keys::super, ImGuiMod_Super, super_down_);

  const std::string_view text = input.text_input();
  std::size_t index = 0;
  while (index < text.size()) {
    const char32_t codepoint = decode_utf8(text, index);
    if (codepoint != 0) {
      io.AddInputCharacter(codepoint);
    }
  }
}

void input_bridge::reset() noexcept {
  keys_down_.reset();
  mouse_down_.reset();
  control_down_ = false;
  shift_down_ = false;
  alt_down_ = false;
  super_down_ = false;
}

} // namespace detail
} // namespace ui
} // namespace cyberverse
