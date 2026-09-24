#include "sdl_window.hpp"

#include <SDL3/SDL.h>

namespace manmenmi::window::detail {
namespace {
Status make_platform_error(std::string_view message) {
    return Status{Error{ErrorCode::platform_error, std::string{message}}};
}
} // namespace

SurfaceRegistry& SurfaceRegistry::instance() {
    static SurfaceRegistry registry;
    return registry;
}

SurfaceToken SurfaceRegistry::register_window(SDL_Window* window) {
    if (window == nullptr) {
        return 0U;
    }

    std::lock_guard lock(mutex_);
    auto existing = by_window_.find(window);
    if (existing != by_window_.end()) {
        return existing->second;
    }

    const SurfaceToken token = next_token_++;
    by_window_[window] = token;
    by_token_[token] = window;
    return token;
}

void SurfaceRegistry::unregister_window(SDL_Window* window) {
    if (window == nullptr) {
        return;
    }

    std::lock_guard lock(mutex_);
    auto existing = by_window_.find(window);
    if (existing == by_window_.end()) {
        return;
    }

    const auto token = existing->second;
    by_window_.erase(existing);
    by_token_.erase(token);
}

SDL_Window* SurfaceRegistry::resolve(SurfaceToken token) const noexcept {
    if (token == 0U) {
        return nullptr;
    }

    std::lock_guard lock(mutex_);
    const auto existing = by_token_.find(token);
    if (existing == by_token_.end()) {
        return nullptr;
    }
    return existing->second;
}

bool SurfaceRegistry::contains(SurfaceToken token) const noexcept {
    return resolve(token) != nullptr;
}

bool SurfaceRegistry::contains_window(SDL_Window* window) const noexcept {
    if (window == nullptr) {
        return false;
    }
    std::lock_guard lock(mutex_);
    return by_window_.find(window) != by_window_.end();
}

SurfaceToken register_surface(SDL_Window* window) {
    return SurfaceRegistry::instance().register_window(window);
}

SDL_Window* resolve_surface_token(const SurfaceToken& token) noexcept {
    return SurfaceRegistry::instance().resolve(token);
}

void unregister_surface(SDL_Window* window) {
    SurfaceRegistry::instance().unregister_window(window);
}

SdlWindow::SdlWindow(const WindowConfig& config, Logger& logger)
    : logger_(&logger),
      window_(nullptr),
      token_(0),
      headless_(false),
      visible_(config.visible),
      close_requested_(false),
      width_(config.width),
      height_(config.height) {
        if (!SDL_Init(SDL_INIT_VIDEO)) {
        logger_->write(LogLevel::warning, LogCategory::window,
            std::string{"SDL video init failed: "} + SDL_GetError() + "; native window unavailable");
        return;
    }

    const auto flags = config.resizeable ? static_cast<Uint32>(SDL_WINDOW_RESIZABLE) : 0U;
    const auto title = config.title.empty() ? "MANMENMI" : config.title.c_str();
    window_ = SDL_CreateWindow(title, static_cast<int>(config.width), static_cast<int>(config.height), flags);
    if (window_ == nullptr) {
        logger_->write(LogLevel::warning, LogCategory::window,
            std::string{"SDL window creation failed: "} + SDL_GetError() + "; native window unavailable");
        SDL_Quit();
        return;
    }

    token_ = register_surface(window_);
    headless_ = false;
    if (!config.visible) {
        SDL_HideWindow(window_);
        visible_ = false;
    } else {
        SDL_ShowWindow(window_);
        visible_ = true;
    }
}

SdlWindow::~SdlWindow() {
    if (window_ != nullptr) {
        unregister_surface(window_);
        SDL_DestroyWindow(window_);
        window_ = nullptr;
    }
    token_ = 0;
    SDL_Quit();
}

Status SdlWindow::show() {
    if (window_ == nullptr) {
        return Status{Error{ErrorCode::not_initialized, "native window has not been created"}};
    }
    SDL_ShowWindow(window_);
    visible_ = true;
    return success();
}

Status SdlWindow::hide() {
    if (window_ == nullptr) {
        return Status{Error{ErrorCode::not_initialized, "native window has not been created"}};
    }
    SDL_HideWindow(window_);
    visible_ = false;
    return success();
}

Status SdlWindow::resize(std::uint32_t width, std::uint32_t height) {
    if (window_ == nullptr) {
        return Status{Error{ErrorCode::not_initialized, "native window has not been created"}};
    }
    if (width == 0 || height == 0) {
        return Status{Error{ErrorCode::invalid_argument, "window dimensions must be non-zero"}};
    }
    if (!SDL_SetWindowSize(window_, static_cast<int>(width), static_cast<int>(height))) {
        return Status{Error{ErrorCode::resize_failed, std::string{"SDL resize failed: "} + SDL_GetError()}};
    }
    width_ = width;
    height_ = height;
    return success();
}

Status SdlWindow::poll_events() {
    if (window_ == nullptr) {
        return Status{Error{ErrorCode::not_initialized, "native window has not been created"}};
    }
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        switch (event.type) {
        case SDL_EVENT_QUIT:
        case SDL_EVENT_WINDOW_CLOSE_REQUESTED:
            close_requested_ = true;
            break;
        case SDL_EVENT_WINDOW_RESIZED:
            width_ = static_cast<std::uint32_t>(event.window.data1);
            height_ = static_cast<std::uint32_t>(event.window.data2);
            break;
        case SDL_EVENT_WINDOW_MINIMIZED:
            visible_ = false;
            break;
        case SDL_EVENT_WINDOW_RESTORED:
            visible_ = true;
            break;
        default:
            break;
        }
    }
    return success();
}

bool SdlWindow::should_close() const noexcept {
    return close_requested_;
}

Extent SdlWindow::size() const noexcept {
    return Extent{width_, height_};
}

bool SdlWindow::is_visible() const noexcept {
    return visible_;
}

SurfaceToken SdlWindow::native_surface_token() const noexcept {
    return token_;
}

Result<std::unique_ptr<Window>> create_window(const WindowConfig& config, Logger& logger) {
    auto window = std::make_unique<SdlWindow>(config, logger);
    if (!window->valid()) {
        return Result<std::unique_ptr<Window>>{Error{ErrorCode::platform_error,
            "SDL3 window creation failed"}};
    }
    return Result<std::unique_ptr<Window>>{std::move(window)};
}
} // namespace manmenmi::window::detail
