#pragma once

#include <cstdint>
#include <string>
#include <manmenmi/core/result.hpp>

namespace manmenmi::window {
struct Extent {
    std::uint32_t width;
    std::uint32_t height;
};
struct Descriptor {
    std::string title;
    Extent logical_size;
};

// M0 contract only. No SDL/native types, surface, GPU object or concrete implementation.
class Window {
public:
    virtual ~Window() = default;
    [[nodiscard]] virtual Extent drawable_extent() const noexcept = 0;
    [[nodiscard]] virtual bool should_close() const noexcept = 0;
    virtual Status poll_events() = 0;
};
} // namespace manmenmi::window