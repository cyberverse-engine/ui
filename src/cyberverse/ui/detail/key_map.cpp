/**
 * @file key_map.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief platform::key_id to ImGuiKey mapping table
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/detail/key_map.hpp"

namespace cyberverse {
namespace ui {
namespace detail {

namespace {

constexpr key_binding g_key_bindings[] = {
  // Printable keys
  { platform::key_id::key_space, ImGuiKey_Space },
  { platform::key_id::key_apostrophe, ImGuiKey_Apostrophe },
  { platform::key_id::key_comma, ImGuiKey_Comma },
  { platform::key_id::key_minus, ImGuiKey_Minus },
  { platform::key_id::key_period, ImGuiKey_Period },
  { platform::key_id::key_slash, ImGuiKey_Slash },
  { platform::key_id::key_0, ImGuiKey_0 },
  { platform::key_id::key_1, ImGuiKey_1 },
  { platform::key_id::key_2, ImGuiKey_2 },
  { platform::key_id::key_3, ImGuiKey_3 },
  { platform::key_id::key_4, ImGuiKey_4 },
  { platform::key_id::key_5, ImGuiKey_5 },
  { platform::key_id::key_6, ImGuiKey_6 },
  { platform::key_id::key_7, ImGuiKey_7 },
  { platform::key_id::key_8, ImGuiKey_8 },
  { platform::key_id::key_9, ImGuiKey_9 },
  { platform::key_id::key_semicolon, ImGuiKey_Semicolon },
  { platform::key_id::key_equal, ImGuiKey_Equal },
  { platform::key_id::key_a, ImGuiKey_A },
  { platform::key_id::key_b, ImGuiKey_B },
  { platform::key_id::key_c, ImGuiKey_C },
  { platform::key_id::key_d, ImGuiKey_D },
  { platform::key_id::key_e, ImGuiKey_E },
  { platform::key_id::key_f, ImGuiKey_F },
  { platform::key_id::key_g, ImGuiKey_G },
  { platform::key_id::key_h, ImGuiKey_H },
  { platform::key_id::key_i, ImGuiKey_I },
  { platform::key_id::key_j, ImGuiKey_J },
  { platform::key_id::key_k, ImGuiKey_K },
  { platform::key_id::key_l, ImGuiKey_L },
  { platform::key_id::key_m, ImGuiKey_M },
  { platform::key_id::key_n, ImGuiKey_N },
  { platform::key_id::key_o, ImGuiKey_O },
  { platform::key_id::key_p, ImGuiKey_P },
  { platform::key_id::key_q, ImGuiKey_Q },
  { platform::key_id::key_r, ImGuiKey_R },
  { platform::key_id::key_s, ImGuiKey_S },
  { platform::key_id::key_t, ImGuiKey_T },
  { platform::key_id::key_u, ImGuiKey_U },
  { platform::key_id::key_v, ImGuiKey_V },
  { platform::key_id::key_w, ImGuiKey_W },
  { platform::key_id::key_x, ImGuiKey_X },
  { platform::key_id::key_y, ImGuiKey_Y },
  { platform::key_id::key_z, ImGuiKey_Z },
  { platform::key_id::key_left_bracket, ImGuiKey_LeftBracket },
  { platform::key_id::key_backslash, ImGuiKey_Backslash },
  { platform::key_id::key_right_bracket, ImGuiKey_RightBracket },
  { platform::key_id::key_grave_accent, ImGuiKey_GraveAccent },

  // Editing and navigation
  { platform::key_id::key_escape, ImGuiKey_Escape },
  { platform::key_id::key_enter, ImGuiKey_Enter },
  { platform::key_id::key_tab, ImGuiKey_Tab },
  { platform::key_id::key_backspace, ImGuiKey_Backspace },
  { platform::key_id::key_insert, ImGuiKey_Insert },
  { platform::key_id::key_delete, ImGuiKey_Delete },
  { platform::key_id::key_right, ImGuiKey_RightArrow },
  { platform::key_id::key_left, ImGuiKey_LeftArrow },
  { platform::key_id::key_down, ImGuiKey_DownArrow },
  { platform::key_id::key_up, ImGuiKey_UpArrow },
  { platform::key_id::key_page_up, ImGuiKey_PageUp },
  { platform::key_id::key_page_down, ImGuiKey_PageDown },
  { platform::key_id::key_home, ImGuiKey_Home },
  { platform::key_id::key_end, ImGuiKey_End },

  // Locks and system keys
  { platform::key_id::key_caps_lock, ImGuiKey_CapsLock },
  { platform::key_id::key_scroll_lock, ImGuiKey_ScrollLock },
  { platform::key_id::key_num_lock, ImGuiKey_NumLock },
  { platform::key_id::key_print_screen, ImGuiKey_PrintScreen },
  { platform::key_id::key_pause, ImGuiKey_Pause },

  // Function keys
  { platform::key_id::key_f1, ImGuiKey_F1 },
  { platform::key_id::key_f2, ImGuiKey_F2 },
  { platform::key_id::key_f3, ImGuiKey_F3 },
  { platform::key_id::key_f4, ImGuiKey_F4 },
  { platform::key_id::key_f5, ImGuiKey_F5 },
  { platform::key_id::key_f6, ImGuiKey_F6 },
  { platform::key_id::key_f7, ImGuiKey_F7 },
  { platform::key_id::key_f8, ImGuiKey_F8 },
  { platform::key_id::key_f9, ImGuiKey_F9 },
  { platform::key_id::key_f10, ImGuiKey_F10 },
  { platform::key_id::key_f11, ImGuiKey_F11 },
  { platform::key_id::key_f12, ImGuiKey_F12 },

  // Modifiers
  { platform::key_id::key_left_shift, ImGuiKey_LeftShift },
  { platform::key_id::key_left_control, ImGuiKey_LeftCtrl },
  { platform::key_id::key_left_alt, ImGuiKey_LeftAlt },
  { platform::key_id::key_left_super, ImGuiKey_LeftSuper },
  { platform::key_id::key_right_shift, ImGuiKey_RightShift },
  { platform::key_id::key_right_control, ImGuiKey_RightCtrl },
  { platform::key_id::key_right_alt, ImGuiKey_RightAlt },
  { platform::key_id::key_right_super, ImGuiKey_RightSuper },
  { platform::key_id::key_menu, ImGuiKey_Menu },
};

} // namespace

const key_binding* key_bindings() noexcept { return g_key_bindings; }

std::size_t key_binding_count() noexcept { return sizeof(g_key_bindings) / sizeof(g_key_bindings[0]); }

} // namespace detail
} // namespace ui
} // namespace cyberverse
