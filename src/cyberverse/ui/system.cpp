/**
 * @file system.cpp
 * @author simon2077s (simon2077s@gmail.com)
 * @brief Dear ImGui + DiligentEngine UI runtime implementation
 * @version 0.3
 * @date 2026-10-27
 *
 * @copyright Copyright (c) 2026
 *
 */
#include "cyberverse/ui/detail/system_impl.hpp"

#include <algorithm>
#include <cmath>
#include <stdexcept>
#include <system_error>
#include <utility>

#include <DeviceContext.h>
#include <ImGuiImplDiligent.hpp>
#include <SwapChain.h>

#include "cyberverse/platform/window.hpp"
#include "cyberverse/render_core/frame_context.hpp"
#include "cyberverse/render_core/render_device.hpp"
#include "cyberverse/ui/detail/frame_phase.hpp"
#include "cyberverse/ui/detail/layer_access.hpp"
#include "cyberverse/ui/detail/z_order.hpp"
#include "cyberverse/ui/layer.hpp"
#include "cyberverse/ui/window_context.hpp"

namespace cyberverse {
namespace ui {

namespace {

constexpr double minimum_delta_time = 1.0 / 1000.0;

#ifndef _WIN32
void on_platform_ime_data(ImGuiContext* /*context*/, ImGuiViewport* viewport, ImGuiPlatformImeData* data) {
  if (viewport == nullptr || data == nullptr) {
    return;
  }
  auto* window = static_cast<platform::window*>(viewport->PlatformHandle);
  if (window == nullptr) {
    return;
  }
  // ImGui reports screen coordinates; platform backends want window-content
  // coordinates. The main viewport position is the window's screen origin.
  window->set_ime_caret(platform::ime_caret{ data->InputPos.x - viewport->Pos.x,
                                             data->InputPos.y - viewport->Pos.y,
                                             data->InputLineHeight,
                                             data->WantVisible });
}
#endif

[[nodiscard]] float sanitize_scale(float value) noexcept {
  return (std::isfinite(value) && value > 0.0f) ? value : 1.0f;
}

[[nodiscard]] float sanitize_font_size(float value) noexcept {
  return (std::isfinite(value) && value > 0.0f) ? value : 16.0f;
}

[[nodiscard]] int sanitize_oversample(int value) noexcept { return value > 0 ? value : 1; }

[[nodiscard]] double sanitize_delta_time(double value) noexcept {
  return (std::isfinite(value) && value > 0.0) ? value : minimum_delta_time;
}

/**
 * @brief RAII guard for one managed top-level window
 */
rect sanitize_window_rect(const rect& requested, const rect& viewport) noexcept {
  if (!std::isfinite(requested.x) || !std::isfinite(requested.y) || !std::isfinite(requested.width) ||
      !std::isfinite(requested.height) || requested.width <= 0.0f || requested.height <= 0.0f) {
    return viewport;
  }

  const float x = std::clamp(requested.x, 0.0f, viewport.width);
  const float y = std::clamp(requested.y, 0.0f, viewport.height);
  const float width = std::min(requested.width, viewport.width - x);
  const float height = std::min(requested.height, viewport.height - y);
  if (width <= 0.0f || height <= 0.0f) {
    return viewport;
  }
  return rect{ x, y, width, height };
}

class managed_window_scope final {
public:
  managed_window_scope(const char* name, ImGuiWindowFlags flags, std::string_view id, const rect& bounds) {
    ImGui::SetNextWindowPos(ImVec2{ bounds.x, bounds.y }, ImGuiCond_Always);
    ImGui::SetNextWindowSize(ImVec2{ bounds.width, bounds.height }, ImGuiCond_Always);
    visible_ = ImGui::Begin(name, nullptr, flags);
    if (visible_) {
      ImGui::PushID(id.data(), id.data() + id.size());
      pushed_ = true;
    }
  }

  ~managed_window_scope() {
    if (pushed_) {
      ImGui::PopID();
    }
    ImGui::End();
  }

  managed_window_scope(const managed_window_scope&) = delete;
  managed_window_scope& operator=(const managed_window_scope&) = delete;

  [[nodiscard]] bool visible() const noexcept { return visible_; }

private:
  bool visible_ = false;
  bool pushed_ = false;
};

} // namespace

system::impl::impl(const system_config& config)
    : config_{ config }, device_{ config.device }, window_{ config.window }, headless_{ config.headless } {
  if (!headless_ && (device_ == nullptr || window_ == nullptr)) {
    throw std::invalid_argument(
        "cyberverse::ui::system: a non-headless system requires both a render device and a platform window");
  }

  if (window_ != nullptr) {
    const glm::vec2 window_scale = window_->framebuffer_scale();
    scale_ = sanitize_scale(window_scale.x);
  }

  create_context();
  try {
    apply_theme(config_.theme);
    load_fonts();
    if (!headless_) {
      // ImGui renders into the swap-chain back buffer, not into a texture
      // created from the requested device format. Diligent may replace the
      // requested color format with the surface-supported format (for example
      // RGBA8 -> BGRA8 on MoltenVK), so the renderer PSO must use the actual
      // swap-chain formats or every ImGui draw reports an RTV mismatch.
      Diligent::TEXTURE_FORMAT color_format = device_->desc().color_format;
      Diligent::TEXTURE_FORMAT depth_format = device_->desc().depth_format;
      if (Diligent::ISwapChain* swap_chain = device_->swap_chain()) {
        const Diligent::SwapChainDesc& swap_chain_desc = swap_chain->GetDesc();
        color_format = swap_chain_desc.ColorBufferFormat;
        depth_format = swap_chain_desc.DepthBufferFormat;
      }
      Diligent::ImGuiDiligentCreateInfo create_info{ device_->device(), color_format, depth_format };
      renderer_ = std::make_unique<Diligent::ImGuiDiligentRenderer>(create_info);
    }
  } catch (...) {
    destroy_context();
    throw;
  }
}

system::impl::~impl() {
  layers_.clear();
  draw_order_.clear();
  layer_signature_.clear();
  destroy_context();
}

void system::impl::create_context() {
  context_ = ImGui::CreateContext();
  if (context_ == nullptr) {
    throw std::runtime_error("cyberverse::ui::system: failed to create the Dear ImGui context");
  }
  ImGui::SetCurrentContext(context_);

  ImGuiIO& io = ImGui::GetIO();
  io.IniFilename = nullptr;
  io.LogFilename = nullptr;
  io.BackendPlatformName = headless_ ? "cyberverse-ui (headless)" : "cyberverse-ui";

  if (config_.keyboard_nav) {
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
  }

  if (config_.persist_ini) {
    ini_path_ = config_.ini_path.string();
    if (!ini_path_.empty()) {
      io.IniFilename = ini_path_.c_str();
    }
  }

  ImGui::GetStyle().WindowMinSize = ImVec2{ 1.0f, 1.0f };
  input_.reset();
  install_ime_hook();
}

void system::impl::install_ime_hook() {
  if (window_ == nullptr) {
    return;
  }

  ImGuiViewport* viewport = ImGui::GetMainViewport();
  if (viewport == nullptr) {
    return;
  }

  viewport->PlatformHandleRaw = window_->native_handle().window;
  viewport->PlatformHandle = window_;

#ifndef _WIN32
  ImGui::GetPlatformIO().Platform_SetImeDataFn = &on_platform_ime_data;
#endif
}

void system::impl::destroy_context() noexcept {
  if (context_ == nullptr) {
    detail::set_frame_in_progress(false);
    return;
  }

  ImGui::SetCurrentContext(context_);
  if (frame_active_) {
    ImGui::EndFrame();
    frame_active_ = false;
  }
  device_context_ = nullptr;

  if (renderer_ != nullptr) {
    renderer_->InvalidateDeviceObjects();
    renderer_.reset();
  }

  ImGui::DestroyContext(context_);
  context_ = nullptr;
  detail::set_frame_in_progress(false);
}

void system::impl::apply_theme(theme_kind theme) {
  switch (theme) {
  case theme_kind::light:
    ImGui::StyleColorsLight();
    break;
  case theme_kind::dark:
  default:
    ImGui::StyleColorsDark();
    break;
  }
  ImGui::GetStyle().WindowMinSize = ImVec2{ 1.0f, 1.0f };
}

const ImWchar* system::impl::ranges_for(const font_config& config) {
  switch (config.range) {
  case glyph_range::chinese_simplified_common:
    return ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon();
  case glyph_range::custom: {
    std::vector<char32_t> glyphs = config.glyphs;
    std::sort(glyphs.begin(), glyphs.end());
    glyphs.erase(std::unique(glyphs.begin(), glyphs.end()), glyphs.end());

    std::vector<ImWchar> ranges;
    for (const char32_t glyph : glyphs) {
      if (glyph == 0 || glyph > 0x10FFFF) {
        continue;
      }
      if constexpr (sizeof(ImWchar) == 2) {
        if (glyph > 0xFFFF) {
          continue;
        }
      }
      ranges.push_back(static_cast<ImWchar>(glyph));
      ranges.push_back(static_cast<ImWchar>(glyph));
    }
    ranges.push_back(0);
    custom_ranges_.push_back(std::move(ranges));
    return custom_ranges_.back().data();
  }
  case glyph_range::default_latin:
  default:
    return ImGui::GetIO().Fonts->GetGlyphRangesDefault();
  }
}

void system::impl::load_fonts() {
  ImGuiIO& io = ImGui::GetIO();
  ImFontAtlas* atlas = io.Fonts;

  font_loaded_.clear();
  custom_ranges_.clear();
  custom_ranges_.reserve(config_.fonts.size());

  ImFont* first_loaded = nullptr;

  for (std::size_t index = 0; index < config_.fonts.size(); ++index) {
    const font_config& config = config_.fonts[index];
    const bool is_default_source = index == 0;
    bool loaded = false;
    ImFont* font = nullptr;

    if (!config.path.empty()) {
      std::error_code error;
      const bool exists = std::filesystem::exists(config.path, error) && !error;
      if (exists) {
        ImFontConfig font_options;
        font_options.MergeMode = config.merge && first_loaded != nullptr;
        font_options.OversampleH = sanitize_oversample(config.oversample_h);
        font_options.OversampleV = sanitize_oversample(config.oversample_v);
        font_options.PixelSnapH = config.pixel_snap;
        font_options.SizePixels = sanitize_font_size(config.size);
        font = atlas->AddFontFromFileTTF(
            config.path.string().c_str(), sanitize_font_size(config.size), &font_options, ranges_for(config));
        loaded = font != nullptr;
      }
    } else if (is_default_source) {
      ImFontConfig font_options;
      font_options.OversampleH = sanitize_oversample(config.oversample_h);
      font_options.OversampleV = sanitize_oversample(config.oversample_v);
      font_options.PixelSnapH = config.pixel_snap;
      font_options.SizePixels = sanitize_font_size(config.size);
      font = atlas->AddFontDefault(&font_options);
      loaded = font != nullptr;
    }

    const std::string id = config.id.empty() ? std::string{ "font." } + std::to_string(index) : config.id;
    font_loaded_[id] = loaded;
    if (loaded && first_loaded == nullptr) {
      first_loaded = font;
    }
  }

  if (first_loaded == nullptr) {
    ImFont* fallback = atlas->AddFontDefault();
    if (fallback == nullptr) {
      throw std::runtime_error("cyberverse::ui::system: failed to create the default Dear ImGui font");
    }
    first_loaded = fallback;
    font_loaded_["default"] = true;
  }

  io.FontDefault = first_loaded;

  if (headless_) {
    // Headless has no renderer and therefore no RendererHasTextures support.
    // Build the atlas eagerly and give it a test-only texture identifier.
    atlas->Build();
    atlas->SetTexID(static_cast<ImTextureID>(1));
  }
}

layer& system::impl::add_layer(std::unique_ptr<layer> layer) {
  detail::require_between_frames("add a UI layer");
  if (layer == nullptr) {
    throw std::invalid_argument("cyberverse::ui::system: layer must not be null");
  }
  if (find_layer(layer->name()) != nullptr) {
    throw std::invalid_argument(std::string{ "cyberverse::ui::system: duplicate layer name '" } +
                                std::string{ layer->name() } + "'");
  }
  layers_.push_back(std::move(layer));
  return *layers_.back();
}

bool system::impl::remove_layer(layer& layer) noexcept {
  if (detail::frame_in_progress()) {
    return false;
  }
  const auto iterator = std::find_if(layers_.begin(), layers_.end(), [&layer](const auto& candidate) {
    return candidate.get() == &layer;
  });
  if (iterator == layers_.end()) {
    return false;
  }
  layers_.erase(iterator);
  return true;
}

void system::impl::clear_layers() {
  detail::require_between_frames("clear UI layers");
  layers_.clear();
  draw_order_.clear();
  layer_signature_.clear();
}

layer* system::impl::find_layer(std::string_view name) noexcept {
  for (const auto& layer : layers_) {
    if (layer != nullptr && layer->name() == name) {
      return layer.get();
    }
  }
  return nullptr;
}

bool system::impl::has_visible_content() const noexcept {
  for (const auto& layer : layers_) {
    if (layer != nullptr && layer->visible() && layer->has_visible_windows()) {
      return true;
    }
  }
  return false;
}

bool system::impl::begin_frame(const render_core::frame_context& frame, double delta_time) {
  if (context_ == nullptr || frame_active_) {
    return false;
  }

  ImGui::SetCurrentContext(context_);

  if (!headless_) {
    if (renderer_ == nullptr || frame.device_context() == nullptr || frame.color_target() == nullptr) {
      return false;
    }
  }

  float framebuffer_width = 0.0f;
  float framebuffer_height = 0.0f;

  if (window_ != nullptr) {
    const int window_width = window_->width();
    const int window_height = window_->height();
    if (window_width <= 0 || window_height <= 0) {
      return false;
    }
    framebuffer_width = static_cast<float>(window_width);
    framebuffer_height = static_cast<float>(window_height);
    if (config_.enable_dpi_scaling && !manual_scale_) {
      scale_ = sanitize_scale(window_->framebuffer_scale().x);
    }
  } else {
    const render_core::viewport& viewport = frame.current_viewport();
    if (!viewport.is_valid()) {
      return false;
    }
    framebuffer_width = viewport.width;
    framebuffer_height = viewport.height;
  }

  const float logical_width = framebuffer_width / scale_;
  const float logical_height = framebuffer_height / scale_;
  if (!(logical_width > 0.0f) || !(logical_height > 0.0f)) {
    return false;
  }

  device_context_ = frame.device_context();
  display_width_ = logical_width;
  display_height_ = logical_height;

  if (!headless_) {
    // UI is a self-contained overlay pass: bind the frame's color/depth targets
    // and start with a full-target scissor. A previous pass may have left other
    // targets or a per-window scissor bound.
    Diligent::ITextureView* targets[1] = { frame.color_target() };
    device_context_->SetRenderTargets(
        1, targets, frame.depth_target(), Diligent::RESOURCE_STATE_TRANSITION_MODE_TRANSITION);
    device_context_->SetScissorRects(0, nullptr, 0, 0);
  }

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2{ logical_width, logical_height };
  io.DisplayFramebufferScale = ImVec2{ scale_, scale_ };
  io.DeltaTime = static_cast<float>(sanitize_delta_time(delta_time));
  update_input();

  if (!headless_) {
    renderer_->NewFrame(static_cast<Diligent::Uint32>(framebuffer_width),
                        static_cast<Diligent::Uint32>(framebuffer_height),
                        Diligent::SURFACE_TRANSFORM_IDENTITY);
  }

  ImGui::NewFrame();
  frame_active_ = true;
  ++frame_index_;
  detail::set_frame_in_progress(true);
  return true;
}

bool system::impl::begin_frame(float display_width, float display_height, double delta_time) {
  if (context_ == nullptr || frame_active_ || !headless_) {
    return false;
  }
  if (!(display_width > 0.0f) || !(display_height > 0.0f) || !std::isfinite(display_width) ||
      !std::isfinite(display_height)) {
    return false;
  }

  ImGui::SetCurrentContext(context_);
  device_context_ = nullptr;
  display_width_ = display_width;
  display_height_ = display_height;

  ImGuiIO& io = ImGui::GetIO();
  io.DisplaySize = ImVec2{ display_width, display_height };
  io.DisplayFramebufferScale = ImVec2{ scale_, scale_ };
  io.DeltaTime = static_cast<float>(sanitize_delta_time(delta_time));
  update_input();

  ImGui::NewFrame();
  frame_active_ = true;
  ++frame_index_;
  detail::set_frame_in_progress(true);
  return true;
}

void system::impl::update_input() {
  if (window_ == nullptr) {
    return;
  }
  input_.update(ImGui::GetIO(), window_->input_state());
}

void system::impl::rebuild_draw_order() {
  bool changed = layer_signature_.size() != layers_.size();
  if (!changed) {
    for (std::size_t index = 0; index < layers_.size(); ++index) {
      if (layer_signature_[index].first != layers_[index].get() ||
          layer_signature_[index].second != layers_[index]->order()) {
        changed = true;
        break;
      }
    }
  }
  if (!changed) {
    return;
  }

  layer_signature_.clear();
  draw_order_.clear();
  layer_signature_.reserve(layers_.size());
  draw_order_.reserve(layers_.size());
  for (const auto& layer : layers_) {
    layer_signature_.emplace_back(layer.get(), layer->order());
    draw_order_.push_back(layer.get());
  }
  std::stable_sort(draw_order_.begin(), draw_order_.end(), [](const layer* left, const layer* right) {
    return left->order() < right->order();
  });
}

ImGuiWindowFlags system::impl::window_flags(const window& content) const {
  ImGuiWindowFlags flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                           ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar |
                           ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing |
                           ImGuiWindowFlags_NoBringToFrontOnFocus;

  const window_feature features = content.features();
  if (has_feature(features, window_feature::title_bar)) {
    flags &= ~ImGuiWindowFlags_NoTitleBar;
  }
  if (has_feature(features, window_feature::scrollbar)) {
    flags &= ~ImGuiWindowFlags_NoScrollbar;
  }
  if (has_feature(features, window_feature::transparent)) {
    flags |= ImGuiWindowFlags_NoBackground;
  }
  if (has_feature(features, window_feature::no_input)) {
    flags |= ImGuiWindowFlags_NoInputs;
  }
  if (has_feature(features, window_feature::focus_on_appearing)) {
    flags &= ~ImGuiWindowFlags_NoFocusOnAppearing;
  }
  if (has_feature(features, window_feature::bring_to_front_on_focus)) {
    flags &= ~ImGuiWindowFlags_NoBringToFrontOnFocus;
  }
  return flags;
}

void system::impl::build_window_name(const layer& layer, const window& content) {
  const std::string_view title = content.title();
  const std::string_view id = content.id();

  window_name_.clear();
  window_name_.append(title.data(), title.size());
  window_name_ += "###";
  window_name_ += layer.name();
  window_name_ += '/';
  window_name_.append(id.data(), id.size());
}

void system::impl::draw(const system& owner) {
  if (context_ == nullptr || !frame_active_) {
    return;
  }

  ImGui::SetCurrentContext(context_);

  try {
    rebuild_draw_order();

    ImGuiIO& io = ImGui::GetIO();
    const ImVec2 display = io.DisplaySize;

    for (layer* layer : draw_order_) {
      if (layer == nullptr || !layer->visible() || !layer->has_visible_windows()) {
        continue;
      }

      layer_window_names_.clear();

      for (auto& content : detail::layer_access::windows(*layer)) {
        if (content == nullptr || !content->visible()) {
          continue;
        }
        if (content->title().find("###") != std::string_view::npos) {
          throw std::invalid_argument("cyberverse::ui::system: a window title must not contain \"###\"");
        }

        build_window_name(*layer, *content);
        layer_window_names_.push_back(window_name_);

        const rect viewport{ 0.0f, 0.0f, display.x, display.y };
        const std::optional<rect> requested = content->window_rect(viewport);
        const rect bounds = requested.has_value() ? sanitize_window_rect(*requested, viewport) : viewport;
        managed_window_scope scope{ window_name_.c_str(), window_flags(*content), content->id(), bounds };
        if (scope.visible()) {
          window_context context{ owner, *layer, bounds, static_cast<double>(io.DeltaTime), frame_index_, scale_ };
          content->draw(context);
        }
      }

      // Raise this layer's windows in window order so the last window wins, then
      // continue with the next (higher) layer.
      for (const std::string& name : layer_window_names_) {
        detail::bring_window_to_front(name.c_str());
      }
    }
  } catch (...) {
    abort_frame();
    throw;
  }
}

void system::impl::end_frame() {
  if (context_ == nullptr || !frame_active_) {
    return;
  }

  ImGui::SetCurrentContext(context_);

  try {
    ImGui::Render();

    if (!headless_ && renderer_ != nullptr && device_context_ != nullptr) {
      renderer_->RenderDrawData(device_context_, ImGui::GetDrawData());
      device_context_->SetScissorRects(0, nullptr, 0, 0);
    }

    ImGuiIO& io = ImGui::GetIO();
    wants_mouse_ = io.WantCaptureMouse;
    wants_keyboard_ = io.WantCaptureKeyboard;

    frame_active_ = false;
    device_context_ = nullptr;
    detail::set_frame_in_progress(false);
  } catch (...) {
    abort_frame();
    throw;
  }
}

void system::impl::abort_frame() noexcept {
  if (context_ != nullptr) {
    ImGui::SetCurrentContext(context_);
    if (frame_active_) {
      ImGui::EndFrame();
    }
  }
  frame_active_ = false;
  device_context_ = nullptr;
  detail::set_frame_in_progress(false);
}

void system::impl::reload_fonts() {
  detail::require_between_frames("reload UI fonts");
  if (context_ == nullptr) {
    return;
  }
  ImGui::SetCurrentContext(context_);
  ImGui::GetIO().Fonts->Clear();
  load_fonts();
}

bool system::impl::font_loaded(std::string_view id) const noexcept {
  const auto iterator = font_loaded_.find(std::string{ id });
  return iterator != font_loaded_.end() && iterator->second;
}

void system::impl::set_theme(theme_kind theme) {
  detail::require_between_frames("change the UI theme");
  if (context_ == nullptr) {
    return;
  }
  ImGui::SetCurrentContext(context_);
  apply_theme(theme);
}

void system::impl::set_scale(float scale) {
  detail::require_between_frames("change the UI scale");
  if (!(std::isfinite(scale) && scale > 0.0f)) {
    throw std::invalid_argument("cyberverse::ui::system: scale must be a finite positive number");
  }
  scale_ = scale;
  manual_scale_ = true;
}

ImGuiIO& system::impl::io() noexcept {
  if (context_ != nullptr) {
    ImGui::SetCurrentContext(context_);
  }
  return ImGui::GetIO();
}

ImGuiStyle& system::impl::style() noexcept {
  if (context_ != nullptr) {
    ImGui::SetCurrentContext(context_);
  }
  return ImGui::GetStyle();
}

system::system(std::unique_ptr<impl> impl) : impl_{ std::move(impl) } {}

system::~system() = default;

std::unique_ptr<system> system::create(const system_config& config) {
  auto implementation = std::make_unique<impl>(config);
  return std::unique_ptr<system>(new system(std::move(implementation)));
}

layer& system::add_layer(std::unique_ptr<layer> layer) { return impl_->add_layer(std::move(layer)); }

bool system::remove_layer(layer& layer) noexcept { return impl_->remove_layer(layer); }

void system::clear_layers() { impl_->clear_layers(); }

layer* system::find_layer(std::string_view name) noexcept { return impl_->find_layer(name); }

bool system::has_visible_content() const noexcept { return impl_->has_visible_content(); }

bool system::begin_frame(const render_core::frame_context& frame, double delta_time) {
  return impl_->begin_frame(frame, delta_time);
}

bool system::begin_frame(float display_width, float display_height, double delta_time) {
  return impl_->begin_frame(display_width, display_height, delta_time);
}

void system::draw() { impl_->draw(*this); }

void system::end_frame() { impl_->end_frame(); }

bool system::frame_active() const noexcept { return impl_->frame_active(); }

bool system::wants_mouse() const noexcept { return impl_->wants_mouse(); }

bool system::wants_keyboard() const noexcept { return impl_->wants_keyboard(); }

void system::reload_fonts() { impl_->reload_fonts(); }

bool system::font_loaded(std::string_view id) const noexcept { return impl_->font_loaded(id); }

void system::set_theme(theme_kind theme) { impl_->set_theme(theme); }

void system::set_scale(float scale) { impl_->set_scale(scale); }

float system::scale() const noexcept { return impl_->scale(); }

ImGuiContext* system::imgui_context() noexcept { return impl_->imgui_context(); }

ImGuiIO& system::io() noexcept { return impl_->io(); }

ImGuiStyle& system::style() noexcept { return impl_->style(); }

} // namespace ui
} // namespace cyberverse
