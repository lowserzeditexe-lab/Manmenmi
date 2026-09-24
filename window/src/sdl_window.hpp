#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_map>

#include <SDL3/SDL.h>

#include <manmenmi/core/log.hpp>
#include <manmenmi/window/window.hpp>

namespace manmenmi::window::detail {
class SurfaceRegistry final {
public:
    static SurfaceRegistry& instance();

    [[nodiscard]] SurfaceToken register_window(SDL_Window* window);
    void unregister_window(SDL_Window* window);
    [[nodiscard]] SDL_Window* resolve(SurfaceToken token) const noexcept;
    [[nodiscard]] bool contains(SurfaceToken token) const noexcept;
    [[nodiscard]] bool contains_window(SDL_Window* window) const noexcept;

private:
    mutable std::mutex mutex_;
    std::uint64_t next_token_ = 1U;
    std::unordered_map<SurfaceToken, SDL_Window*> by_token_;
    std::unordered_map<SDL_Window*, SurfaceToken> by_window_;
};

[[nodiscard]] SurfaceToken register_surface(SDL_Window* window);
[[nodiscard]] SDL_Window* resolve_surface_token(const SurfaceToken& token) noexcept;
void unregister_surface(SDL_Window* window);

class SdlWindow final : public manmenmi::window::Window {
public:
    SdlWindow(const WindowConfig& config, Logger& logger);
    ~SdlWindow() override;

    Status show() override;
    Status hide() override;
    Status resize(std::uint32_t width, std::uint32_t height) override;
    Status poll_events() override;

    [[nodiscard]] bool should_close() const noexcept override;
    [[nodiscard]] Extent size() const noexcept override;
    [[nodiscard]] bool is_visible() const noexcept override;
    [[nodiscard]] SurfaceToken native_surface_token() const noexcept override;

    [[nodiscard]] bool valid() const noexcept { return window_ != nullptr; }

private:
    Logger* logger_;
    SDL_Window* window_;
    SurfaceToken token_;
    bool headless_;
    bool visible_;
    bool close_requested_;
    std::uint32_t width_;
    std::uint32_t height_;
};

[[nodiscard]] Result<std::unique_ptr<Window>> create_window(const WindowConfig& config, Logger& logger);
} // namespace manmenmi::window::detail
