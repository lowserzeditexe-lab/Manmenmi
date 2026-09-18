#pragma once

#include <memory>
#include <manmenmi/core/backend.hpp>

namespace manmenmi::graphics {
// M2 names reserved; resource semantics and IR deliberately NOT invented in M0.
class Queue;
class CommandBuffer;
class Buffer;
class Texture;
class Shader;
class Pipeline;
class Framebuffer;
class Sync;
class Swapchain;

class Device {
public:
    virtual ~Device() = default;
    [[nodiscard]] virtual manmenmi::Backend backend() const noexcept = 0;
};

class Backend {
public:
    virtual ~Backend() = default;
    [[nodiscard]] virtual manmenmi::Backend kind() const noexcept = 0;
    [[nodiscard]] virtual Result<std::unique_ptr<Device>> create_device() = 0;
};
} // namespace manmenmi::graphics