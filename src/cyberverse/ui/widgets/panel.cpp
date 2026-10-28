/**
 * @file panel.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Styled flexbox container implementation
 * @version 0.4
 * @date 2026-10-28
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/widgets/panel.hpp"

#include "cyberverse/ui/widgets/detail/draw_utils.hpp"
#include "cyberverse/ui/widgets/detail/layout_types.hpp"
#include "cyberverse/ui/widgets/detail/style_resolver.hpp"

namespace cyberverse {
namespace ui {
namespace widgets {

void panel::draw(detail::draw_context& context) {
  const detail::resolved_style style =
      detail::resolve_style(detail::style_kind::panel, context.appearance, style_override_);
  detail::draw_container(context.draw_list, bounds_, style);
}

} // namespace widgets
} // namespace ui
} // namespace cyberverse
