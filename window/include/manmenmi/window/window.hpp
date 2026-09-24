#pragma once

#include <cstdint>
#include <string>
#include <manmenmi/core/result.hpp>

namespace manmenmi {
using SurfaceToken = std::uint64_t;

namespace window {
struct WindowConfig {
    std::string title;
    std::uint32_t width;
    std::uint32_t height;
    bool resizeable = true;
    bool visible = true;
};

struct Extent {
    std::uint32_t width;
    std::uint32_t height;
};

enum class WindowEvent {
    none,
    close_requested,
    resized,
    minimized,
    restored
};

class Window {
public:
    virtual ~Window() = default;

    virtual Status show() = 0;
    virtual Status hide() = 0;
    virtual Status resize(std::uint32_t width, std::uint32_t height) = 0;
    virtual Status poll_events() = 0;

    [[nodiscard]] virtual bool should_close() const noexcept = 0;
    [[nodiscard]] virtual Extent size() const noexcept = 0;
    [[nodiscard]] virtual bool is_visible() const noexcept = 0;
    [[nodiscard]] virtual SurfaceToken native_surface_token() const noexcept = 0;
};
} // namespace window
} // namespace manmenmi