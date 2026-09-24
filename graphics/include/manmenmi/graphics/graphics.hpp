#pragma once

#include <cstdint>
#include <manmenmi/core/backend.hpp>
#include <manmenmi/core/log.hpp>
#include <manmenmi/core/result.hpp>
#include <manmenmi/window/window.hpp>
#include <memory>
#include <span>
#include <string_view>

namespace manmenmi {
namespace graphics {
class Swapchain;
}

enum class BackendSelection { automatic, vulkan, dx12, opengl };

struct BackendCapabilities {
  bool presentable = false;
  bool render_target_supported = false;
  bool multi_buffering = false;
  bool msaa_supported = false;
  bool separate_graphics_queue = false;

  std::uint32_t min_width = 1;
  std::uint32_t min_height = 1;
  std::uint32_t max_backbuffers = 3;

  std::string_view adapter_name;
  std::string_view vendor_name;
};

enum class BufferUsage { transfer_source, transfer_destination, vertex };

struct BufferDescriptor {
  std::uint64_t size = 0U;
  BufferUsage usage = BufferUsage::transfer_source;
};

enum class TextureFormat { rgba8_unorm };

enum class TextureUsage { render_target };

// This is a backend-neutral ownership contract, not a physical GPU memory
// layout.
enum class TextureLayout { opaque_render_target };

struct TextureDescriptor {
  std::uint32_t width = 0U;
  std::uint32_t height = 0U;
  TextureFormat format = TextureFormat::rgba8_unorm;
  TextureUsage usage = TextureUsage::render_target;
  TextureLayout layout = TextureLayout::opaque_render_target;
};

class Buffer {
public:
  virtual ~Buffer() = default;

  virtual Status upload(std::span<const std::uint8_t> data) = 0;
};

class Texture {
public:
  virtual ~Texture() = default;
};

class Pipeline {
public:
  virtual ~Pipeline() = default;
};

class CommandBuffer {
public:
  virtual ~CommandBuffer() = default;

  virtual Status begin() = 0;
  virtual Status copy_buffer(Buffer &source, Buffer &destination,
                             std::uint64_t source_offset,
                             std::uint64_t destination_offset,
                             std::uint64_t size) = 0;
  virtual Status begin_render_pass(graphics::Swapchain &swapchain) = 0;
  virtual Status begin_render_pass(Texture &texture) = 0;
  virtual Status set_pipeline(Pipeline &pipeline) = 0;
  virtual Status bind_vertex_buffer(Buffer &buffer, std::uint64_t offset) = 0;
  virtual Status draw(std::uint32_t vertex_count,
                      std::uint32_t first_vertex) = 0;
  virtual Status end_render_pass() = 0;
  virtual Status close() = 0;
  virtual Status reset() = 0;
};

namespace graphics {
class Queue {
public:
  virtual ~Queue() = default;

  virtual Status submit() = 0;
  virtual Status submit(CommandBuffer &command_buffer) = 0;
  virtual Status present() = 0;
  virtual Status wait_idle() = 0;
  virtual Status reset() = 0;
};

class Swapchain {
public:
  virtual ~Swapchain() = default;

  virtual Status acquire_next_image() = 0;

  [[nodiscard]] virtual std::uint32_t image_count() const noexcept = 0;
  [[nodiscard]] virtual std::uint32_t current_image_index() const noexcept = 0;

  virtual Status resize(std::uint32_t width, std::uint32_t height) = 0;
  virtual Status recreate() = 0;
};

class Device {
public:
  virtual ~Device() = default;

  [[nodiscard]] virtual BackendSelection backend() const noexcept = 0;
  [[nodiscard]] virtual BackendCapabilities capabilities() const noexcept = 0;
  virtual Status wait_idle() = 0;
  [[nodiscard]] virtual Result<std::unique_ptr<Buffer>>
  create_buffer(const BufferDescriptor &descriptor) = 0;
  [[nodiscard]] virtual Result<std::unique_ptr<Texture>>
  create_texture(const TextureDescriptor &descriptor) = 0;
  [[nodiscard]] virtual Result<std::unique_ptr<CommandBuffer>>
  create_command_buffer() = 0;
  [[nodiscard]] virtual Result<std::unique_ptr<Pipeline>>
  create_triangle_pipeline() = 0;
};

class GraphicsFactory {
public:
  virtual ~GraphicsFactory() = default;

  [[nodiscard]] virtual Result<std::unique_ptr<Device>>
  create_device(BackendSelection selection, Logger &logger) = 0;

  [[nodiscard]] virtual Result<std::unique_ptr<Swapchain>>
  create_swapchain(Device &device, const SurfaceToken &surface_token,
                   std::uint32_t width, std::uint32_t height) = 0;
};

[[nodiscard]] std::unique_ptr<GraphicsFactory> make_graphics_factory();
} // namespace graphics
} // namespace manmenmi
